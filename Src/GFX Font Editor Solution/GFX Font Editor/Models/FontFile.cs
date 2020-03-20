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

		[JsonProperty("items")]
		public IEnumerable<Glyph> Items { get; set; }
	}
}
