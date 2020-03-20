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
