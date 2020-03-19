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
using Windows.ApplicationModel.Resources;

namespace GfxFontEditor.Models
{
	public class CodeFactory
	{
		public CodeFactory(IEnumerable<Glyph> items)
		{
			this.Items = items;
		}

		protected IEnumerable<Glyph> Items { get; set; }

		public Task<string> CreateSourceCode(string className)
		{
			StringBuilder returnValue = new StringBuilder();

			// ***
			// *** Get the file header.
			// ***
			ResourceLoader resourceLoader = ResourceLoader.GetForCurrentView();
			string copyright = resourceLoader.GetString("Copyright").Replace("{Year}", DateTime.Now.Year.ToString());

			// ***
			// *** Get the file header.
			// ***
			string header = resourceLoader.GetString("Header")
								.Replace("{FontName}", className)
								.Replace("{DateCreated}", DateTime.Now.ToLongDateString())
								.Replace("{Version}", "1.0.0");

			// ***
			// *** Build the bitmap array items.
			// ***
			IDictionary<int, string> bitmaps = this.GetBitmapItems();

			// ***
			// *** Get the max length for proper spacing.
			// ***
			int maxBitmapLength = bitmaps.Select(t => t.Value).Max(t => t.Length);

			// ***
			// *** Create the glyphs array items.
			// ***
			IDictionary<int, string> glyphs = this.GetGlyphArrayItems();

			// ***
			// *** Get the max length for proper spacing.
			// ***
			int maxGlyphLength = glyphs.Select(t => t.Value).Max(t => t.Length);

			// ***
			// *** Assemble the code.
			// ***
			returnValue.AppendLine(copyright);
			returnValue.AppendLine();
			returnValue.AppendLine(header);
			returnValue.AppendLine();
			returnValue.AppendLine($"#define {className.ToUpper()}_USE_EXTENDED 0");
			returnValue.AppendLine();
			returnValue.AppendLine($"const uint8_t {className}Bitmaps[] PROGMEM =");
			returnValue.AppendLine("{");

			foreach (KeyValuePair<int, string> item in bitmaps)
			{
				Glyph glyph = this.GetGlyph(item.Key);
				returnValue.Append($"\t{item.Value}{this.CommaOrSpace(bitmaps, item)}".PadRight(maxBitmapLength + 4, ' '));
				returnValue.AppendLine(glyph.CodeComment);

				if (glyph.AsciiCode == 0x7e)
				{
					returnValue.AppendLine($"#if ({className.ToUpper()}_USE_EXTENDED)");
				}
			}

			returnValue.AppendLine("#endif");
			returnValue.AppendLine("};");
			returnValue.AppendLine();
			returnValue.AppendLine("/* {offset, width, height, advance cursor, x offset, y offset} */");
			returnValue.AppendLine($"const GFXglyph {className}Glyphs[] PROGMEM =");
			returnValue.AppendLine("{");

			foreach (KeyValuePair<int, string> item in glyphs)
			{
				Glyph glyph = this.GetGlyph(item.Key);
				returnValue.Append($"\t{item.Value}{this.CommaOrSpace(glyphs, item)}".PadRight(maxGlyphLength + 4, ' '));
				returnValue.AppendLine(glyph.CodeComment);

				if (glyph.AsciiCode == 0x7e)
				{
					returnValue.AppendLine($"#if ({className.ToUpper()}_USE_EXTENDED)");
				}
			}

			returnValue.AppendLine("#endif");
			returnValue.AppendLine("};");
			returnValue.AppendLine();

			returnValue.AppendLine($"const GFXfont {className} PROGMEM =");
			returnValue.AppendLine("{");
			returnValue.AppendLine($"\t(uint8_t*){className}Bitmaps,");
			returnValue.AppendLine($"\t(GFXglyph*){className}Glyphs,");
			returnValue.AppendLine($"\t0x{this.Items.Min(t => t.AsciiCode):x2}, /* First ASCII Character */");
			returnValue.AppendLine($"\t0x{this.Items.Max(t => t.AsciiCode):x2}, /* Last ASCII Character */");
			returnValue.AppendLine($"\t0x{(this.Items.Max(t => t.Height) + 2):x2} /* Vertical Spacing */");
			returnValue.AppendLine("};");

			return Task.FromResult(returnValue.ToString());
		}

		protected IDictionary<int, string> GetBitmapItems()
		{
			IDictionary<int, string> returnValue = new Dictionary<int, string>();

			foreach (Glyph item in this.Items.OrderBy(t => t.AsciiCode))
			{
				StringBuilder bitmap = new StringBuilder();

				for (int i = 0; i < item.FontBitmap.Count(); i++)
				{
					byte b = item.FontBitmap.ElementAt(i);
					bitmap.Append($"0x{b:x2}");

					if (i < item.FontBitmap.Count() - 1)
					{
						bitmap.Append(", ");
					}
				}

				returnValue.Add(item.AsciiCode, bitmap.ToString());
			}

			return returnValue;
		}

		protected IDictionary<int, string> GetGlyphArrayItems()
		{
			IDictionary<int, string> returnValue = new Dictionary<int, string>();

			foreach (Glyph item in this.Items)
			{
				returnValue.Add(item.AsciiCode, $"{item.Display}");
			}

			return returnValue;
		}

		protected Glyph GetGlyph(int asciiCode)
		{
			return this.Items.OrderBy(t => t.AsciiCode).Where(t => t.AsciiCode == asciiCode).Single();
		}

		protected string CommaOrSpace<T>(IEnumerable<T> items, T item)
		{
			string returnValue = String.Empty;

			if (item.Equals(items.Last()))
			{
				returnValue = "  ";
			}
			else
			{
				returnValue = ", ";
			}

			return returnValue;
		}
	}
}
