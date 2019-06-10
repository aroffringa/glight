#ifndef COLOR_H
#define COLOR_H

class Color
{
	public:
		Color(unsigned char red, unsigned char green, unsigned char blue) :
			_red(red), _green(green), _blue(blue)
		{
		}
		unsigned char Red() const { return _red; }
		unsigned char Green() const { return _green; }
		unsigned char Blue() const { return _blue; }
		
		Color operator*(unsigned char rhs) const
		{
			return Color(_red*rhs/255, _green*rhs/255, _blue*rhs/255);
		}

		static Color Gray(unsigned char intensity) { return Color(intensity, intensity, intensity); }
		
		static Color Black() { return Color(0, 0, 0); }
		static Color White() { return Color(255, 255, 255); }
		static Color WhiteOrange() { return Color(255, 192, 128); }
		static Color RedC() { return Color(255, 0, 0); }
		static Color Orange() { return Color(255, 128, 0); }
		static Color OrangeGreen() { return Color(170, 255, 0); }
		static Color GreenC() { return Color(0, 255, 0); }
		static Color GreenBlue() { return Color(0, 255, 255); }
		static Color GreenYellow() { return Color(128, 255, 0); }
		static Color BlueC() { return Color(0, 0, 255); }
		static Color LBlue() { return Color(0, 255, 255); }
		static Color BlueYellow() { return Color(192, 192, 255); }
		static Color Yellow() { return Color(255, 255, 0); }
		static Color YellowPurple() { return Color(255, 128, 255); }
		static Color Purple() { return Color(255, 0, 255); }
		static Color PurpleBlue() { return Color(128, 0, 255); }
		static Color PurpleWhite() { return Color(255, 128, 255); }
		
		static Color H20Color(unsigned value)
		{
			if(value <= 10)
				return White();
			else if(value <= 21)
				return WhiteOrange();
			else if(value <= 32)
				return Orange();
			else if(value <= 43)
				return OrangeGreen();
			else if(value <= 54)
				return GreenC();
			else if(value <= 65)
				return GreenBlue();
			else if(value <= 76)
				return BlueC();
			else if(value <= 87)
				return BlueYellow();
			else if(value <= 98)
				return Yellow();
			else if(value <= 109)
				return YellowPurple();
			else if(value <= 120)
				return Purple();
			else if(value <= 127)
				return PurpleWhite();
			else
				return White();
		}
		
		static Color ADJMacroColor(unsigned value)
		{
			/*
			if(value < 8)
				return Black();
			else if(value < 12)
				return RedC();
			else if(value < 18)
				return GreenC();
			else if(value < 24)
				return BlueC();
			else if(value < 30)
				return Yellow();
			else if(value < 36)
				return Purple();
			else if(value < 42)
				return LBlue();
			else if(value < 48)
				return White();
			else if(value < 54)
				return GreenYellow();
			else if(value < 60)
				return PurpleBlue();
			else if(value < 66)
				return Color(128, 128, 0);
			else if(value < 72)
				return Color(128, 0, 128);
			else if(value < 78)
				return Color(0, 128, 128);
			else if(value < 84)
				return Color(192, 128, 0);
			else if(value < 90)
				return Color(192, 0, 128);
			else if(value < 96)
				return Color(192, 128, 128);
			else if(value < 102)
				return Color(0, 192, 128);
			else if(value < 108)
				return Color(128, 192, 0);
			else if(value < 114)
				return Color(128, 192, 128);
			else if(value < 120)
				return Color(0, 128, 192);
			else if(value < 126)
				return Color(128, 0, 192);
			else if(value < 132)
				return Color(128, 128, 192);
			else if(value < 138)
				return Color(96, 144, 0);
			else if(value < 144)
				return Color(96, 0, 144);
			else if(value < 150)
				return Color(96, 144, 96);
			else if(value < 156)
				return Color(128, 144, 144);
			else if(value < 162)
				return Color(0, 96, 144);
			else if(value < 168)
				return Color(144, 96, 144);
			else if(value < 174)
				return Color(0, 144, 96);
			else if(value < 180)
				return Color(144, 0, 96);
			else if(value < 186)
				return Color(144, 144, 96);
			else if(value < 192)
				return Color(44, 180, 0);
			else if(value < 198)
				return Color(44, 0, 180);
			else if(value < 204)
				return Color(44, 180, 180);
			else if(value < 210)
				return Color(180, 44, 0);
			else if(value < 216)
				return Color(0, 44, 180);
			else if(value < 222)
				return Color(180, 44, 180);
			else if(value < 228)
				return Color(0, 180, 44);
			else if(value < 234)
				return Color(180, 0, 44);
			else if(value < 240)
				return Color(0, 180, 44);
			else if(value < 246)
				return Color(20, 96, 0);
			else if(value < 252)
				return Color(20, 0, 96);
			else
				return Color(20, 96, 96);
			*/
			if(value < 132)
			{
				if(value < 66)
				{
					if(value < 36)
					{
						if(value < 18)
						{
							if(value < 8)
								return Black(); // < 8
							else if(value < 12)
								return RedC(); // < 12
							else
								return GreenC(); // < 18
						}
						else { // >= 18
							if(value < 24)
								return BlueC(); // < 24
							else if(value < 30)
								return Yellow(); // < 30
							else
								return Purple(); // < 36
						}
					} // < 66, >= 36
					else {
						if(value < 54)
						{
							if(value < 42)
								return LBlue(); // < 42
							else if(value < 48)
								return White(); // < 48
							else 
								return GreenYellow(); // < 54
						}
						else { // >= 54
							if(value < 60)
								return PurpleBlue(); // < 60
							else
								return Color(128, 128, 0); // < 66
						}
					}
				}
				else { // < 132, >= 66
					if(value < 102)
					{
						if(value < 84)
						{
							if(value < 72)
								return Color(128, 0, 128); // < 72
							else if(value < 78)
								return Color(0, 128, 128); // < 78
							else 
								return Color(192, 128, 0); // < 84
						}
						else {
							if(value < 90)
								return Color(192, 0, 128); // < 90
							else if(value < 96)
								return Color(192, 128, 128); // < 96
							else
								return Color(0, 192, 128); // < 102
						}
					}
					else { // < 132, >= 102
						if(value < 120)
						{
							if(value < 108)
								return Color(128, 192, 0); // < 108
							else if(value < 114)
								return Color(128, 192, 128); // < 114
							else
								return Color(0, 128, 192); // < 120
						}
						else {
							if(value < 126)
								return Color(128, 0, 192); // < 126
							else
								return Color(128, 128, 192); // < 132
						}
					}
				}
			}
			else { // >= 132
				if(value < 198)
				{
					if(value < 168)
					{
						if(value < 150)
						{
							if(value < 138)
								return Color(96, 144, 0); // < 138
							else if(value < 144)
								return Color(96, 0, 144); // < 144
							else
								return Color(96, 144, 96); // < 150
						}
						else {
							if(value < 156)
								return Color(144, 96, 0); // < 156
							else if(value < 162)
								return Color(0, 96, 144); // < 160
							else
								return Color(144, 96, 144); // < 168
						}
					}
					else { // >= 168, < 196
						if(value < 186)
						{
							if(value < 174)
								return Color(0, 144, 96); // < 174
							else if(value < 180)
								return Color(144, 0, 96); // < 180
							else
								return Color(144, 144, 96); // < 186
						}
						else if(value < 192)
							return Color(44, 180, 0); // < 192
						else
							return Color(44, 0, 180); // < 198
					}
				}
				else { // >= 198
					if(value < 228)
					{
						if(value < 216)
						{
							if(value < 204)
								return Color(44, 180, 180); // < 204
							else if(value < 210)
								return Color(180, 44, 0); // < 210
							else
								return Color(0, 44, 180); // < 216
						}
						else if(value < 222)
							return Color(180, 44, 180); // < 222
						else
							return Color(0, 180, 44); // < 228
					}
					else { // >= 228
						if(value < 246)
						{
							if(value < 234)
								return Color(180, 0, 44); // < 234
							else if(value < 240)
								return Color(0, 180, 44); // < 240
							else
								return Color(20, 96, 0); // < 246
						}
						else if(value < 252)
							return Color(20, 0, 96); // < 252
						else
							return Color(20, 96, 96);
					}
				}
			}
		}
	private:
		unsigned char _red, _green, _blue;	
};

#endif // COLOR_H
