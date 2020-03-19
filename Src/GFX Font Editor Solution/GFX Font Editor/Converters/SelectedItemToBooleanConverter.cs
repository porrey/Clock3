using System;
using GfxFontEditor.Models;
using Windows.UI.Xaml.Data;

namespace GfxFontEditor.Converters
{
	public sealed class SelectedItemToBooleanConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, string language)
		{
			bool returnValue = false;

			if (value is Glyph glyph)
			{
				returnValue = (glyph != null);
			}

			return returnValue;
		}

		public object ConvertBack(object value, Type targetType, object parameter, string language)
		{
			throw new NotSupportedException();
		}
	}
}