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

		public FontFile CurrentFile { get; set; }

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

		private string _fontName = "New Font";
		public string FontName
		{
			get
			{
				return _fontName;
			}
			set
			{
				this.SetProperty(ref _fontName, Path.GetFileNameWithoutExtension(value));
			}
		}

		public StorageFile StorageFile { get; set; }

		protected void CreateGrid(FontFile font)
		{
			this.BitmapGrid.RowDefinitions.Clear();
			this.BitmapGrid.ColumnDefinitions.Clear();
			this.BitmapGrid.Children.Clear();

			if (font != null && font.Items != null && font.Items.Count() > 0)
			{
				// ***
				// *** Create the columns.
				// ***
				for (int c = 0; c < font.FontWidth; c++)
				{
					ColumnDefinition col = new ColumnDefinition() { Width = new GridLength(1, GridUnitType.Star) };
					this.BitmapGrid.ColumnDefinitions.Add(col);
				}

				// ***
				// *** Create the rows.
				// ***
				for (int r = 0; r < font.FontHeight; r++)
				{
					RowDefinition row = new RowDefinition() { Height = new GridLength(1, GridUnitType.Star) };
					this.BitmapGrid.RowDefinitions.Add(row);
				}

				// ***
				// *** Add the border elements.
				// ***
				int index = 0;

				for (int r = 0; r < font.FontHeight; r++)
				{
					for (int c = 0; c < font.FontWidth; c++)
					{
						double left = c == 0 ? 0 : 1;
						double top = r == 0 ? 0 : 1;

						Border b = new Border()
						{
							BorderThickness = new Thickness(left, top, 0, 0),
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
				// ***
				// *** Simulate the GFS drawing of this glyph. The cursor starts
				// *** at the bottom.
				// ***
				int baseline = this.CurrentFile.FontHeight - 1;
				int top = baseline + glyph.yOffset + 1;
				int bottom = top + glyph.Height - 1;

				for (int i = 0; i < this.CurrentFile.FontHeight; i++)
				{
					if (i >= top && i <= bottom)
					{
						int byteIndex = i - (this.CurrentFile.FontHeight + glyph.yOffset);

						if (byteIndex >= 0 && byteIndex < glyph.FontBitmap.Count())
						{
							byte b = glyph.FontBitmap[byteIndex];
							int x = glyph.xOffset;

							// ***
							// *** Check each bit.
							// ***
							for (int bi = glyph.Width - 1; bi >= 0; bi--)
							{
								byte mask = (byte)(1 << bi);
								bool isOn = (byte)(b & mask) != 0;
								this.SetPixel(i, x, isOn, isOn ? Colors.Red : Colors.White);

								x++;
							}
						}
					}
					else
					{
						for (int j = 0; j < glyph.Width; j++)
						{
							this.SetPixel(i, j, false, Colors.LightGray);
						}
					}
				}

				this.SetPixel(baseline, glyph.xOffset + glyph.xAdvance, false, Color.FromArgb(32, 0, 0, 255));
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
				CodeImport fontFile = new CodeImport(file);
				this.CurrentFile = await fontFile.Import();
				this.CreateGrid(this.CurrentFile);
				this.Items.AddRange(this.CurrentFile.Items);
				this.FontName = file.Name;
				this.StorageFile = null;
				this.RaisePropertyChanged(nameof(this.Items));
			}
		}

		private async void ExportFont(object sender, RoutedEventArgs e)
		{
			FileSavePicker picker = new FileSavePicker()
			{
				SuggestedStartLocation = PickerLocationId.DocumentsLibrary,
				SuggestedFileName = this.FontName,
				DefaultFileExtension = ".h"
			};

			picker.FileTypeChoices.Add("H", new List<string>() { ".h" });

			StorageFile file = await picker.PickSaveFileAsync();

			if (file != null)
			{
				this.CurrentFile.Items = this.Items.ToArray();
				CodeFactory codeFactory = new CodeFactory(this.CurrentFile);
				string code = await codeFactory.CreateSourceCode(this.FontName);
				await FileIO.WriteTextAsync(file, code);
			}
		}

		private async void SaveFont(object sender, RoutedEventArgs e)
		{
			StorageFile file = null;

			if (this.StorageFile == null)
			{
				FileSavePicker picker = new FileSavePicker()
				{
					SuggestedStartLocation = PickerLocationId.DocumentsLibrary,
					SuggestedFileName = this.FontName,
					DefaultFileExtension = ".json"
				};

				picker.FileTypeChoices.Add("JSON", new List<string>() { ".json" });

				file = await picker.PickSaveFileAsync();
			}
			else
			{
				file = this.StorageFile;
			}

			if (file != null)
			{
				this.CurrentFile.Items = this.Items.ToArray();
				string json = JsonConvert.SerializeObject(this.CurrentFile, Formatting.Indented);
				await FileIO.WriteTextAsync(file, json);
				this.FontName = file.Name;
				this.StorageFile = file;
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
				this.CurrentFile = JsonConvert.DeserializeObject<FontFile>(json);
				this.CreateGrid(this.CurrentFile);
				this.Items.AddRange(this.CurrentFile.Items);
				this.FontName = file.Name;
				this.StorageFile = file;
				this.RaisePropertyChanged(nameof(this.Items));
			}
		}

		private void NewFont(object sender, RoutedEventArgs e)
		{
			this.Items.Clear();

			this.CurrentFile = new FontFile
			{
				FontWidth = 8,
				FontHeight = 7,
				ExtendedCharacters = false
			};

			for (int i = 32; i <= 0x7E; i++)
			{
				char c = Convert.ToChar(i);

				this.Items.Add(new Glyph()
				{
					Key = Convert.ToString(c),
					BitmapOffset = 0,
					Width = this.CurrentFile.FontWidth,
					Height = this.CurrentFile.FontHeight,
					xAdvance = 5,
					xOffset = 0,
					yOffset = -5,
					AsciiCode = c,
					FontBitmap = new List<byte>() { 0, 0, 0, 0, 0 }
				});

				this.CurrentFile.Items = this.Items.ToArray();
				this.StorageFile = null;
			}

			this.CreateGrid(this.CurrentFile);
			this.FontName = "New Font";
			this.RaisePropertyChanged(nameof(this.Items));
		}

		private List<byte> GetBitmapFromGrid(Glyph glyph)
		{
			List<byte> returnValue = new List<byte>();

			if (glyph != null)
			{
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
					Glyph previousGlyph = this.Items.ElementAt(i - 1);
					glyph.BitmapOffset = previousGlyph.BitmapOffset + previousGlyph.Height;
				}
			}
		}

		private void KeyTextChanged(object sender, TextChangedEventArgs e)
		{
			this.RaisePropertyChanged(nameof(this.SelectedItem.Key));
		}

		private void HeightChanged(object sender, TextChangedEventArgs e)
		{
			this.Reindex();
			this.DrawGlyph(this.SelectedItem);
			this.RaisePropertyChanged(nameof(this.SelectedItem.Height));
		}

		private void xAdvanceChanged(object sender, TextChangedEventArgs e)
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

		private void IncrementHeight(object sender, RoutedEventArgs e)
		{
			if (this.SelectedItem != null && this.SelectedItem.Height < this.CurrentFile.FontHeight)
			{
				this.SelectedItem.Height++;
			}
		}

		private void DecrementHeight(object sender, RoutedEventArgs e)
		{
			if (this.SelectedItem != null && this.SelectedItem.Height > 1)
			{
				this.SelectedItem.Height--;
			}
		}

		private void IncrementxOffset(object sender, RoutedEventArgs e)
		{
			if (this.SelectedItem != null && this.SelectedItem.xOffset < this.CurrentFile.FontWidth)
			{
				this.SelectedItem.xOffset++;
			}
		}

		private void DecrementxOffset(object sender, RoutedEventArgs e)
		{
			if (this.SelectedItem != null && this.SelectedItem.xOffset > 0)
			{
				this.SelectedItem.xOffset--;
			}
		}

		private void IncrementyOffset(object sender, RoutedEventArgs e)
		{
			if (this.SelectedItem != null)
			{
				this.SelectedItem.yOffset++;
			}
		}

		private void DecrementyOffset(object sender, RoutedEventArgs e)
		{
			if (this.SelectedItem != null)
			{
				this.SelectedItem.yOffset--;
			}
		}

		private void IncrementxAdvance(object sender, RoutedEventArgs e)
		{
			if (this.SelectedItem != null)
			{
				this.SelectedItem.xAdvance++;
			}
		}

		private void DecrementxAdvance(object sender, RoutedEventArgs e)
		{
			if (this.SelectedItem != null)
			{
				this.SelectedItem.xAdvance--;
			}
		}
	}
}
