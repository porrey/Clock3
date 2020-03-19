using System;
using System.Collections.ObjectModel;
using System.Linq;
using GfxFontEditor.Models;
using Windows.UI.Xaml.Data;

namespace GfxFontEditor.Converters
{
	public sealed class ItemsCountToBooleanConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, string language)
		{
			bool returnValue = false;

			if (value is ObservableCollection<Glyph> items)
			{
				returnValue = (items.Count() > 0);
			}

			return returnValue;
		}

		public object ConvertBack(object value, Type targetType, object parameter, string language)
		{
			throw new NotSupportedException();
		}
	}
}