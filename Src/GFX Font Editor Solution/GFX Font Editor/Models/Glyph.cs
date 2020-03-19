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
using System.Collections.Generic;
using Newtonsoft.Json;

namespace GfxFontEditor.Models
{
	public class Glyph : BindableObject
	{
		/// <summary>
		/// Name or character represented by this structure.
		/// </summary>
		private string _key;
		[JsonProperty("key")]
		public string Key { get => _key; set => this.SetProperty(ref _key, value); }

		/// <summary>
		/// Character ASCII code
		/// </summary>
		private int _asciiCode;
		[JsonProperty("asciiCode")]
		public int AsciiCode { get => _asciiCode; set => this.SetProperty(ref _asciiCode, value); }

		/// <summary>
		/// Pointer into GFXfont->bitmap.
		/// </summary>
		private int _bitmapOffset;
		[JsonProperty("bitmapOffset")]
		public int BitmapOffset { get => _bitmapOffset; set => this.SetProperty(ref _bitmapOffset, value); }

		/// <summary>
		/// Bitmap dimensions in pixels.
		/// </summary>
		private int _width;
		[JsonProperty("width")]
		public int Width { get => _width; set => this.SetProperty(ref _width, value); }

		/// <summary>
		/// Bitmap dimensions in pixels.
		/// </summary>
		private int _height;
		[JsonProperty("height")]
		public int Height { get => _height; set => this.SetProperty(ref _height, value); }

		/// <summary>
		/// Distance to advance cursor (x axis).
		/// </summary>
		private int _xAdvance;
		[JsonProperty("xAdvance")]
		public int xAdvance { get => _xAdvance; set => this.SetProperty(ref _xAdvance, value); }

		/// <summary>
		/// X dist from cursor position to UL corner.
		/// </summary>
		private int _xOffset;
		[JsonProperty("xOffset")]
		public int xOffset { get => _xOffset; set => this.SetProperty(ref _xOffset, value); }

		/// <summary>
		/// Y dist from cursor position to UL corner.
		/// </summary>
		private int _yOffset;
		[JsonProperty("yOffset")]
		public int yOffset { get => _yOffset; set => this.SetProperty(ref _yOffset, value); }

		/// <summary>
		/// Byte array that defines the character.
		/// </summary>
		[JsonProperty("fontBitmap")]
		public List<byte> FontBitmap { get; set; } = new List<byte>();

		[JsonIgnore]
		public string Display=> $"{{ {this.BitmapOffset}, {this.Width}, {this.Height}, {this.xAdvance}, {this.xOffset}, {this.yOffset} }}";

		[JsonIgnore]
		public string Hex => $"0x{this.AsciiCode:X2}";

		[JsonIgnore]
		public string CodeComment => $"/* 0x{this.AsciiCode:x2} {this.Key} */";

		public override string ToString()
		{
			return $"{this.Key} [{this.BitmapOffset}, {this.Width}, {this.Height}, {this.xAdvance}, {this.xOffset}, {this.yOffset}]";
		}
	}
}
