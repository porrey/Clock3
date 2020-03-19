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
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using GfxFontEditor.Models;
using Newtonsoft.Json;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.UI;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;

namespace GfxFontEditor
{
	public partial class MainPage : BindablePage
	{
		public MainPage()
		{
			this.InitializeComponent();

			//this.Items.Add(new Glyph() { Key = "A", BitmapOffset = 135, Width = 8, Height = 5, xAdvance = 4, xOffset = 0, yOffset = -5, FontBitmap = new List<byte>() { 0x40, 0xA0, 0xE0, 0xA0, 0xA0 } });
			//this.Items.Add(new Glyph() { Key = "B", BitmapOffset = 140, Width = 8, Height = 5, xAdvance = 4, xOffset = 0, yOffset = -5, FontBitmap = new List<byte>() { 0xC0, 0xA0, 0xC0, 0xA0, 0xC0 } });
			//this.Items.Add(new Glyph() { Key = "C", BitmapOffset = 145, Width = 8, Height = 5, xAdvance = 4, xOffset = 0, yOffset = -5, FontBitmap = new List<byte>() { 0x60, 0x80, 0x80, 0x80, 0x60 } });
			//this.Items.Add(new Glyph() { Key = "D", BitmapOffset = 150, Width = 8, Height = 5, xAdvance = 4, xOffset = 0, yOffset = -5, FontBitmap = new List<byte>() { 0xC0, 0xA0, 0xA0, 0xA0, 0xC0 } });
			//this.Items.Add(new Glyph() { Key = ":", BitmapOffset = 150, Width = 8, Height = 3, xAdvance = 2, xOffset = 0, yOffset = -4, FontBitmap = new List<byte>() { 0x80, 0x00, 0x80 } });

			//string file = @"C:\Users\porrd0bl\Downloads\Adafruit-GFX-Library-master\Adafruit-GFX-Library-master\Fonts\TomThumb.h";
			//string daata = File.ReadAllText(file);
		}

		public ObservableCollection<Glyph> Items { get; } = new ObservableCollection<Glyph>();

		private Glyph _selectedItem = null;
		public Glyph SelectedItem
		{
			get
			{
				return _selectedItem;
			}
			set
			{
				this.SetProperty(ref _selectedItem, value);
				this.DrawGlyph(_selectedItem);
			}
		}

		private string _fileName = "New Font";
		public string FileName
		{
			get
			{
				return _fileName;
			}
			set
			{
				this.SetProperty(ref _fileName, Path.GetFileNameWithoutExtension(value));
			}
		}

		public StorageFile File { get; set; }

		protected void CreateGrid(IEnumerable<Glyph> items)
		{
			this.BitmapGrid.RowDefinitions.Clear();
			this.BitmapGrid.ColumnDefinitions.Clear();
			this.BitmapGrid.Children.Clear();

			if (items != null && items.Count() > 0)
			{
				(int rows, int columns) = this.GetGridDimensions(items);

				// ***
				// *** Create the columns.
				// ***
				for (int c = 0; c <= columns; c++)
				{
					ColumnDefinition col = new ColumnDefinition() { Width = new GridLength(50) };
					this.BitmapGrid.ColumnDefinitions.Add(col);
				}

				// ***
				// *** Create the rows.
				// ***
				for (int r = 0; r <= rows; r++)
				{
					RowDefinition col = new RowDefinition() { Height = new GridLength(50) };
					this.BitmapGrid.RowDefinitions.Add(col);
				}

				// ***
				// *** Add the border elements.
				// ***
				int index = 0;

				for (int r = 0; r < rows; r++)
				{
					for (int c = 0; c < columns; c++)
					{
						Border b = new Border()
						{
							BorderThickness = new Thickness(1, 1, c == columns - 1 ? 1 : 0, r == rows - 1 ? 1 : 0),
							BorderBrush = new SolidColorBrush(Color.FromArgb(255, 0xaa, 0xaa, 0xaa))
						};

						b.Tag = new BorderTag() { Row = r, Column = c, IsOn = false };
						b.PointerPressed += this.GridItemClicked;
						b.IsHitTestVisible = true;
						b.SetValue(Grid.ColumnProperty, c);
						b.SetValue(Grid.RowProperty, r);

						this.BitmapGrid.Children.Add(b);

						index++;
					};
				}
			}
		}

		private void GridItemClicked(object sender, PointerRoutedEventArgs e)
		{
			if (sender is Border border)
			{
				if (!((BorderTag)border.Tag).IsOn)
				{
					border.Background = new SolidColorBrush(Colors.Red);
					((BorderTag)border.Tag).IsOn = true;
				}
				else
				{
					border.Background = new SolidColorBrush(Colors.White);
					((BorderTag)border.Tag).IsOn = false;
				}

				this.SelectedItem.FontBitmap = this.GetBitmapFromGrid(this.SelectedItem);
			}
		}

		protected void DrawGlyph(Glyph glyph)
		{
			this.ClearGrid();

			if (glyph != null)
			{
				(int rows, int columns) = this.GetGridDimensions(this.Items);

				// ***
				// *** Simulate the GFS drawing of this glyph. The cursor starts
				// *** at the bottom.
				// ***
				int baseline = rows - 1;
				int x = glyph.xOffset;
				int y = baseline + glyph.yOffset + 1;

				foreach (byte b in glyph.FontBitmap)
				{
					// ***
					// *** Check each bit.
					// ***
					for (int i = glyph.Width - 1; i >= 0; i--)
					{
						byte mask = (byte)(1 << i);
						bool isOn = (byte)(b & mask) != 0;
						this.SetPixel(y, x, isOn, isOn ? Colors.Red : Colors.White);
						x++;
					}

					y++;
					x = glyph.xOffset;
				}

				x = glyph.xOffset + glyph.xAdvance;
				y = baseline;
				this.SetPixel(y, x, false, Color.FromArgb(32, 0, 0, 255));
			}
		}

		protected void ClearGrid()
		{
			foreach (UIElement child in this.BitmapGrid.Children)
			{
				if (child is Border b)
				{
					b.Background = new SolidColorBrush(Colors.White);
					((BorderTag)b.Tag).IsOn = false;
				}
			}
		}

		protected void SetPixel(int row, int column, bool bitOn, Color color)
		{
			Border border = this.GetBorderObject(row, column);

			if (border != null)
			{
				border.Background = new SolidColorBrush(color);
				((BorderTag)border.Tag).IsOn = bitOn;
			}
		}

		protected (int, int) GetGridDimensions(IEnumerable<Glyph> items)
		{
			(int rows, int columns) = (0, 0);

			if (items != null && items.Count() > 0)
			{
				rows = items.Max(t => t.Height);
				columns = items.Max(t => t.Width);
			}

			return (rows, columns);
		}

		protected Border GetBorderObject(int row, int column)
		{
			Border returnValue = (from tbl in this.BitmapGrid.Children
								  where tbl is Border border
								  && ((BorderTag)border.Tag).Column == column
								  && ((BorderTag)border.Tag).Row == row
								  select tbl).SingleOrDefault() as Border;

			return returnValue;
		}

		private async void ImportFont(object sender, RoutedEventArgs e)
		{
			FileOpenPicker picker = new FileOpenPicker
			{
				ViewMode = PickerViewMode.Thumbnail,
				SuggestedStartLocation = PickerLocationId.DocumentsLibrary
			};

			picker.FileTypeFilter.Add(".h");

			StorageFile file = await picker.PickSingleFileAsync();

			if (file != null)
			{
				this.Items.Clear();
				FontFile fontFile = new FontFile(file);
				IEnumerable<Glyph> items = await fontFile.Import();
				this.CreateGrid(items);
				this.Items.AddRange(items);
				this.FileName = file.Name;
				this.File = null;
				this.RaisePropertyChanged(nameof(this.Items));
			}
		}

		private async void ExportFont(object sender, RoutedEventArgs e)
		{
			FileSavePicker picker = new FileSavePicker()
			{
				SuggestedStartLocation = PickerLocationId.DocumentsLibrary,
				SuggestedFileName = this.FileName,
				DefaultFileExtension = ".h"
			};

			picker.FileTypeChoices.Add("H", new List<string>() { ".h" });

			StorageFile file = await picker.PickSaveFileAsync();

			if (this.File != null)
			{
				CodeFactory codeFactory = new CodeFactory(this.Items);
				string code = await codeFactory.CreateSourceCode(this.FileName);
				await FileIO.WriteTextAsync(file, code);
			}
		}

		private async void SaveFont(object sender, RoutedEventArgs e)
		{
			StorageFile file = null;

			if (this.File == null)
			{
				FileSavePicker picker = new FileSavePicker()
				{
					SuggestedStartLocation = PickerLocationId.DocumentsLibrary,
					SuggestedFileName = this.FileName,
					DefaultFileExtension = ".json"
				};

				picker.FileTypeChoices.Add("JSON", new List<string>() { ".json" });

				file = await picker.PickSaveFileAsync();
			}
			else
			{
				file = this.File;
			}

			if (this.File != null)
			{
				string json = JsonConvert.SerializeObject(this.Items.ToArray(), Formatting.Indented);
				await FileIO.WriteTextAsync(file, json);
				this.FileName = file.Name;
				this.File = file;
			}
		}

		private async void OpenFont(object sender, RoutedEventArgs e)
		{
			FileOpenPicker picker = new FileOpenPicker
			{
				ViewMode = PickerViewMode.Thumbnail,
				SuggestedStartLocation = PickerLocationId.DocumentsLibrary
			};

			picker.FileTypeFilter.Add(".json");

			StorageFile file = await picker.PickSingleFileAsync();

			if (file != null)
			{
				this.Items.Clear();
				string json = await FileIO.ReadTextAsync(file);
				IEnumerable<Glyph> items = JsonConvert.DeserializeObject<IEnumerable<Glyph>>(json);
				this.CreateGrid(items);
				this.Items.AddRange(items);
				this.FileName = file.Name;
				this.File = file;
				this.RaisePropertyChanged(nameof(this.Items));
			}
		}

		private void NewFont(object sender, RoutedEventArgs e)
		{
			this.Items.Clear();

			for (int i = 32; i <= 0x7E; i++)
			{
				char c = Convert.ToChar(i);

				this.Items.Add(new Glyph()
				{
					Key = Convert.ToString(c),
					BitmapOffset = 0,
					Width = 8,
					Height = 5,
					xAdvance = 5,
					xOffset = 0,
					yOffset = -4,
					AsciiCode = c,
					FontBitmap = new List<byte>() { 0, 0, 0, 0, 0 }
				});

				this.File = null;
			}

			this.CreateGrid(this.Items);
			this.FileName = "New Font";
			this.RaisePropertyChanged(nameof(this.Items));
		}

		private List<byte> GetBitmapFromGrid(Glyph glyph)
		{
			List<byte> returnValue = new List<byte>();

			if (glyph != null)
			{
				(int rows, int columns) = this.GetGridDimensions(this.Items);

				for (int row = 0; row < glyph.Height; row++)
				{
					byte b = 0;

					for (int column = 0; column < glyph.Width; column++)
					{
						Border border = this.GetBorderObject(row, column);
						byte bit = ((BorderTag)border.Tag).IsOn ? (byte)1 : (byte)0;
						b = (byte)((b << 1) + bit);
					}

					returnValue.Add(b);
				}
			}

			return returnValue;
		}

		private void Reindex()
		{
			for (int i = 0; i < this.Items.Count(); i++)
			{
				if (i == 0)
				{
					Glyph glyph = this.Items.ElementAt(i);
					glyph.BitmapOffset = 0;
				}
				else
				{
					Glyph glyph = this.Items.ElementAt(i);
					Glyph previousGlyph = this.Items.ElementAt(i-1);
					glyph.BitmapOffset = previousGlyph.BitmapOffset + previousGlyph.Height;
				}
			}
		}

		private void KeyTextChanged(object sender, TextChangedEventArgs e)
		{
			this.RaisePropertyChanged(nameof(this.SelectedItem.Key));
		}

		private void WidthChanged(object sender, TextChangedEventArgs e)
		{
			this.Reindex();
			this.RaisePropertyChanged(nameof(this.SelectedItem.Width));
		}

		private void HeightChanged(object sender, TextChangedEventArgs e)
		{
			this.Reindex();
			this.RaisePropertyChanged(nameof(this.SelectedItem.Height));
		}

		private void xAdavanceChanged(object sender, TextChangedEventArgs e)
		{
			this.DrawGlyph(this.SelectedItem);
			this.RaisePropertyChanged(nameof(this.SelectedItem.xAdvance));
		}

		private void xOffsetChanged(object sender, TextChangedEventArgs e)
		{
			this.DrawGlyph(this.SelectedItem);
			this.RaisePropertyChanged(nameof(this.SelectedItem.xOffset));
		}

		private void yOffsetChanged(object sender, TextChangedEventArgs e)
		{
			this.DrawGlyph(this.SelectedItem);
			this.RaisePropertyChanged(nameof(this.SelectedItem.yOffset));
		}
	}
}
