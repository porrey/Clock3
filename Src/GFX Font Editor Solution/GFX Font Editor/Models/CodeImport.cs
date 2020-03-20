// ***
// *** Copyright(C) 2020, Daniel M. Porrey. All rights reserved.
// *** 
// *** This program is free software: you can redistribute it and/or modify
// *** it under the terms of the GNU Lesser General Public License as published
// *** by the Free Software Foundation, either version 3 of the License, or
// *** (at your option) any later version.
// *** 
// *** This program is distributed in the hope that it will be useful,
// *** but WITHOUT ANY WARRANTY; without even the implied warranty of
// *** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *** GNU Lesser General Public License for more details.
// *** 
// *** You should have received a copy of the GNU Lesser General Public License
// *** along with this program. If not, see http://www.gnu.org/licenses/.
// ***
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;

namespace GfxFontEditor.Models
{
	public class CodeImport
	{
		public CodeImport(StorageFile file)
		{
			this.File = file;
		}

		protected StorageFile File { get; set; }

		public async Task<FontFile> Import()
		{
			FontFile returnValue = new FontFile();
			returnValue.Items = new List<Glyph>();

			IList<string> lines = await FileIO.ReadLinesAsync(this.File);

			bool readingBitmapBytes = false;
			bool readingGlyphs = false;
			StringBuilder hexString = new StringBuilder();
			int asc = 0x20;

			foreach (string line in lines)
			{
				if (!readingBitmapBytes && line.Contains("const uint8_t") && line.Contains("PROGMEM = {"))
				{
					readingBitmapBytes = true;
				}
				else if (readingBitmapBytes)
				{
					if (line.Contains("};"))
					{
						readingBitmapBytes = false;
					}
					else if (!line.StartsWith("#"))
					{
						if (line.Contains("/*"))
						{
							int pos = line.IndexOf("/*");
							hexString.Append(line.Substring(0, pos).Trim());
						}
						else
						{
							hexString.Append(line.Trim());
						}
					}
				}
				else if (!readingGlyphs && line.Contains("const GFXglyph") && line.Contains("PROGMEM = {"))
				{
					readingGlyphs = true;
				}
				else if (readingGlyphs)
				{
					if (line.Contains("};"))
					{
						readingGlyphs = false;
					}
					else if (!String.IsNullOrEmpty(line) && !line.StartsWith("#"))
					{
						int startPos = line.IndexOf("{");
						int endPos = line.IndexOf("}");

						string stringValue = line.Substring(startPos + 1, endPos - startPos - 1);
						string[] values = stringValue.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
						char c = Convert.ToChar(asc);

						Glyph glyph = new Glyph()
						{
							Key = Convert.ToString(c),
							BitmapOffset = Convert.ToInt32(values[0]),
							Width = Convert.ToInt32(values[1]),
							Height = Convert.ToInt32(values[2]),
							xAdvance = Convert.ToInt32(values[3]),
							xOffset = Convert.ToInt32(values[4]),
							yOffset = Convert.ToInt32(values[5]),
							AsciiCode = asc
						};

						((IList<Glyph>)returnValue.Items).Add(glyph);

						asc++;
					}
				}
			}

			// ***
			// *** Convert the bitmap array.
			// ***
			byte[] bitmapArray = this.ConvertHexString(hexString.ToString());

			// ***
			// *** Check the byte array count.
			// ***
			int expectedByteCount = returnValue.Items.Sum(t => t.Height);

			if (expectedByteCount == bitmapArray.Count())
			{
				foreach (Glyph glyph in returnValue.Items)
				{
					glyph.FontBitmap.AddRange(bitmapArray.Skip(glyph.BitmapOffset).Take(glyph.Height));
				}
			}

			returnValue.FontWidth = returnValue.Items.Max(t => t.Width);
			returnValue.FontHeight = returnValue.Items.Max(t => t.Height);

			return returnValue;
		}

		protected byte[] ConvertHexString(string hexString)
		{
			string[] hexArray = hexString.ToString().Replace("0x", "").Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries);

			return (from tbl in hexArray
					select Convert.ToByte(tbl.Trim(), 16)).ToArray();
		}
	}
}
