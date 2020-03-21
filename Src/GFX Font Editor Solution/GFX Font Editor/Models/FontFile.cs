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
using Newtonsoft.Json;

namespace GfxFontEditor.Models
{
	public class FontFile
	{
		[JsonProperty("fontHeight")]
		public int FontHeight { get; set; }

		[JsonProperty("fontWidth")]
		public int FontWidth { get; set; }

		[JsonProperty("extendedCharacters")]
		public bool ExtendedCharacters { get; set; }

		[JsonProperty("description")]
		public string Description { get; set; }

		[JsonProperty("majorVersion")]
		public uint MajorVersion { get; set; } = 1;

		[JsonProperty("minorVersion")]
		public uint MinorVersion { get; set; }

		[JsonProperty("dateTimeCreated")]
		public DateTime DateTimeCreated { get; set; } = DateTime.Now;

		[JsonProperty("lastModifiedDateTime")]
		public DateTime LastModifiedDateTime { get; set; }

		[JsonProperty("items")]
		public IEnumerable<Glyph> Items { get; set; }

		public void IncrementVersion()
		{
			this.MinorVersion++;

			if (this.MinorVersion > 99)
			{
				this.MajorVersion++;
				this.MinorVersion = 0;
			}
		}
	}
}
