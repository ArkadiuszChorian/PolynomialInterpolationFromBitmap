#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include "EasyBMP.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
//Implementacja dla linuxa funkcji getch i getche znanych z biblioteki conio.h
static struct termios old, temp;

void initTermios(int echo)
{
  tcgetattr(0, &old);
  temp = old;
  temp.c_lflag &= ~ICANON;
  temp.c_lflag &= echo ? ECHO : ~ECHO;
  tcsetattr(0, TCSANOW, &temp);
}
void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &old);
}
char getch_(int echo)
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}
char getch(void)
{
  return getch_(0);
}
char getche(void)
{
  return getch_(1);
}
#endif // _WIN32

using namespace std;

class Point //Klasa pomocnicza dla wspolrzednych punktow
{
	public:
		double x,y;
		Point(double _x,double _y):x(_x),y(_y){};
};

class DataFromBitmap //Klasa ktorej obiet przechowuje dane o bitmapie
{
    public:
        vector <Point> founded_points; //Kontener zawierajacy znalezione punkty na bitmapie
        vector <Point> filtered_points; //Kontener zawierajacy przefiltrowane punkty wyjsciowe
        int thickness, axis; //Parametr gestosci oraz os
        DataFromBitmap(int _thickness):thickness(100-_thickness){};

        bool IsWhite (RGBApixel pix) //Metoda logiczna mowiaca czy dany pixel jest bialy czy innego koloru
        {
            if (pix.Red==255 && pix.Green==255 && pix.Blue==255)
                return true;
            else
                return false;
        }

        void find_axis(BMP* img) //Metoda znajdujaca os
        {
            int vert = 0;
            int size_of_line = 0;
            int licz = 0;
            bool founded = false;
            while(founded == false)
            {
                for (int i = 0; i < img->TellHeight(); i++)
                {
                    licz++;
                    if ( IsWhite(img->GetPixel(vert,i)) == false )
                    {
                        for (int j = vert; j < img->TellWidth(); j++)
                        {
                            if ( IsWhite(img->GetPixel(j,i)) == false )
                                size_of_line++;
                            if ( size_of_line >= floor( 0.8 * (double)img->TellWidth() ) )
                            {
                                axis = i;
                                founded = true;
                                break;
                            }
                        }
                    }
                }
                vert++;
            }
        }

        int value(int horiz) //Metoda pozycjonujaca prawidlowo os
        {
            return ( -1 * (horiz - axis) );
        }

        void find_points(BMP* img) //Metoda znajdujaca i zapisujaca pixele z bitmapy jako punkty
        {
            for(int i = 0; i < img->TellWidth(); i++)
            {
                int temp = i;
                for(int j = 0; j < img->TellHeight(); j++)
                {
                    if(j == axis)
                    continue;
                    if ( IsWhite(img->GetPixel(i,j)) == false && temp == i)
                    {
                        founded_points.push_back( Point( static_cast<double>(i), static_cast<double>(value(j)) ) );
                        temp++;
                    }
                }
            }
        }

        double sum(int iterator) //Metoda zwracajaca sume wartosci pewnej ilosci punktow (potrzebna w algorytmie 1
        {
            double result = 0;
            for (int i = iterator; i <= iterator+(thickness); i++)
            {
                result += founded_points[i].y;
            }
            return result;
        }

        void set_output_points1() //Algorytm filtracji punktow przez usrednianie
        {
            double middle_value;
            int argument_x = founded_points.front().x;
            for (int i = 0; i < founded_points.size(); i+=(thickness))
            {
                if ( i+thickness >= founded_points.size()-1 )
                {
                    filtered_points.push_back( Point(founded_points.back().x,founded_points.back().y) );
                    break;
                }
                middle_value = sum(i) / (thickness);
                filtered_points.push_back( Point(static_cast<double>(argument_x+floor(thickness/2)),middle_value) );
                argument_x += (thickness);
            }
        }

        void set_output_points2() //Algorytm filtracji punktow przez wybieranie co n-tego punktu
        {
            for (int i = 0; i < founded_points.size(); i+=(thickness))
            {
                if (i+thickness >= founded_points.size()-1)
                {
                    filtered_points.push_back( Point(founded_points.back().x,founded_points.back().y));
                    break;
                }
                filtered_points.push_back( Point(founded_points[i].x,founded_points[i].y) );
            }
        }

        void set_output_points3() //Algorytm filtracji punktow przez minimalna roznice wartosci
        {
            for (int i = 0; i < founded_points.size(); i++)
            {
                if ( abs(founded_points[i].y - founded_points[i+1].y) <=thickness )
                {
                    int k = i;
                    while ( abs(founded_points[i].y - founded_points[k].y) <= thickness )
                    {
                        if ( (k-i+1)%thickness == 0 )
                        {
                            break;
                        }
                        k++;
                    }
                    if ( founded_points[floor((i+k)/2)].x == 0 && i!=0 )
                        break;
                    filtered_points.push_back( Point(founded_points[floor((i+k)/2)].x, founded_points[floor((i+k)/2)].y) );
                    i = k;
                }
                else
                {
                    filtered_points.push_back( Point(founded_points[i].x, founded_points[i].y) );
                }
            }
        }
};

class Polynomial //Klasa charakteryzujaca wielomian i dostarczajaca metod dla jego obslugi
{
    public:
        Polynomial(vector <Point> in, vector <Point> founded, int r):input_points(in),founded_points(founded),rank(r) //Konstruktor argumentowy, przyjmuje gotowy zestaw punktow i rzad
        {
            find_coefficients(); //Wspolczynniki obliczane sa juz podczas konstrukcji obiektu
        }
        vector <double> coefficients; //Kontener zawierajacy wspolczynniki wielomianu
        vector <Point> data_to_write; //Kontener pomocniczy dla przechowywania serii punktow do pozniejszego plotowania
        vector <Point> input_points; //Kontener przechowujacy pierwotnie interpolowane punkty
        vector <Point> founded_points; //Kontener przechowujacy punkty pierwotnie znalezione na bitmapie
        int rank; //Rzad wielomianu i ilosc punktow interpolowanych pierwotnie przez wielomian

        void find_coefficients() //Metoda obliczania wspolczynnikow wielomianu
        {
            vector <Point> temp = input_points;
            double ci;
            int k = input_points.size()-1;

            ci = input_points[0].y;
            coefficients.push_back(ci);

            for (int i = 1; i<=rank; i++) //Algorytm roznic dzielonych dla znalezienia wspolczynnikow
            {
                for(int j = 1; j <=k; j++)
                {
                    ci = (temp[j].y - temp[j-1].y) / (input_points[j+i-1].x - input_points[j-1].x);
                    if (j == 1)
                    {
                        coefficients.push_back(ci);
                    }
                    temp[j-1].y = ci;
                }
                k--;
            }
        }

        double value(double x) //Metoda obliczania wartosci wielomianu w zadanym punkcie
        {
            double value = coefficients[0];
            double temp = 1;
            for (int i = 1; i<coefficients.size(); i++)
            {
                temp = temp * (x - input_points[i-1].x);
                value += coefficients[i] * temp;
            }
            return value;
        }

        void generate_points() //Metoda wygenerowania punktow wielomianu w celu pozniejszego wyplotowania
        {
            for (int i = input_points.front().x; i <= input_points.back().x; i++)
            {
                data_to_write.push_back(Point(i,value(i)));
            }
        }

        void save_to_file() //Metoda zapisu wygenerowanych punktow do pliku w celu pozniejszego wyplotowania
        {
            fstream to_file;
            to_file.open("polynomial.dat", ios::in | ios::out | ios::trunc);

            for (int i = 0; i < data_to_write.size(); i++)
            {
                to_file<<data_to_write[i].x<<" "<<data_to_write[i].y<<endl;
            }
            to_file.close();

            to_file.open("filtered.dat", ios::in | ios::out | ios::trunc);
            for (int i = 0; i < input_points.size(); i++)
            {
                to_file<<input_points[i].x<<" "<<input_points[i].y<<endl;
            }
            to_file.close();

            to_file.open("function.dat", ios::in | ios::out | ios::trunc);

            for (int i = 0; i < founded_points.size(); i++)
            {
                to_file<<founded_points[i].x<<" "<<founded_points[i].y<<endl;
            }
            to_file.close();
        }

        void plot(string cmd, string dir) //Funkcja plotowania z argumentem komendy do gnuplota i sciezki gnuplota
        {
            FILE * gnuplot;

            gnuplot = popen( dir.c_str(),"w" );
            fprintf( gnuplot, cmd.c_str());
            fflush(gnuplot);

            cout<<"Funkcja interpolowana jest wielomianem "<<rank<<" rzedu\n\n";
            cout<<"Aby kontynuowac nacisnij ENTER\n";
            getch();

            pclose(gnuplot);
        }

};

void cls() //Funkcja czyszczenia ekranu zalezna od systemu
{
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif // _WIN32
}

int main()
{
    char key;
    int algorithm_flag = 3;
    string filename = "brak";
    string cmd  = "set terminal wxt\n";
           cmd += "set title 'Wyinterpolowany wielomian'\n";
           cmd += "set xlabel 'x'\n set ylabel 'y'\n set key inside left top\n";
           cmd += "plot \"polynomial.dat\" title 'Wielomian' w l, \"function.dat\" title 'Funkcja z pliku' w l, \"filtered.dat\" title 'Przefiltrowane punkty' w p\n";
    string display = "wielomian, funkcja z pliku i filtrowane punkty";
    string algorithm = "minimalna roznica wartosci";
    #ifdef _WIN32
    string directory = "D:/gnuplot/bin/gnuplot.exe";
    #else
    string directory = "gnuplot";
    #endif // _WIN32
    int thickness = 50;
    BMP* bitmap;
    DataFromBitmap* data;
    Polynomial* interpolation;

    while (key != 27)
    {
        cls();
        cin.clear();
        cin.sync();
        cout<<"Program do interpolacji wielomianowej, funkcji z wykresu w pliku bitmap.\n\n";
        cout<<"1   - Nazwa pliku bitmapowego: "<<filename<<endl;
        cout<<"2   - Gestosc punktow interpolacyjnych: "<<thickness<<endl;
        cout<<"3   - Wyswietlanie na wykresie: "<<display<<endl;
        cout<<"4   - Algorytm filtracji punktow: "<<algorithm<<endl;
        #ifdef _WIN32
        cout<<"5   - Sciezka pakietu gnuplot: "<<directory<<endl;
        #endif // _WIN32
        cout<<"d   - Wyswietl interpolowany wielomian"<<endl;
        cout<<"h   - Pomoc"<<endl;
        cout<<"ESC - Zakonczenie programu "<<endl<<endl;
        key = getch();

        switch (key)
        {
        case 49:
            cls();
            cout<<"Wprowadz nazwe pliku bitmapowego (rozszerzenie zostanie dodane automatycznie):\n";
            cin>>filename;
            filename+=".bmp";
            break;
        case 50:
            cls();
            cout<<"Wprowadz gestosc punktow interpolacyjnych (zakres: 0 = rzadko , 99 = gesto):\n";
            cin>>thickness;
            if( cin.fail() || thickness < 0 || thickness > 99)
            {
                cin.clear();
                cin.sync();
                thickness = 50;
                cout<<"Podales bledna wartosc. Przywrocono wartosc domyslna 50";
                getch();
            }
            break;
        case 51:
            cls();
            cout<<"Wybierz co ma byc wyswietlane na wykresie:\n\n";
            cout<<"1   - Tylko wyinterpolowany wielomian\n";
            cout<<"2   - Wyinterpolowany wielomian i funkcja z pliku bitmapowego\n";
            cout<<"3   - Wyinterpolowany wielomian, funkcja z pliku oraz przefiltrowane punkty\n";
            key = getch();
            switch(key)
            {
            case 49:
                cmd  = "set terminal wxt\n";
                cmd += "set title 'Wyinterpolowany wielomian'\n";
                cmd += "set xlabel 'x'\n set ylabel 'y'\n set key inside left top\n";
                cmd += "plot \"polynomial.dat\" title 'Wielomian' w l\n";
                display = "tylko wielomian";
                break;
            case 50:
                cmd  = "set terminal wxt\n";
                cmd += "set title 'Wyinterpolowany wielomian'\n";
                cmd += "set xlabel 'x'\n set ylabel 'y'\n set key inside left top\n";
                cmd += "plot \"polynomial.dat\" title 'Wielomian' w l, \"function.dat\" title 'Funkcja z pliku' w l\n";
                display = "wielomian i funkcja z pliku";
                break;
            case 51:
                cmd  = "set terminal wxt\n";
                cmd += "set title 'Wyinterpolowany wielomian'\n";
                cmd += "set xlabel 'x'\n set ylabel 'y'\n set key inside left top\n";
                cmd += "plot \"polynomial.dat\" title 'Wielomian' w l, \"function.dat\" title 'Funkcja z pliku' w l, \"filtered.dat\" title 'Przefiltrowane punkty' w p\n";
                display = "wielomian, funkcja z pliku i filtrowane punkty";
                break;
            default:
                cmd  = "set terminal wxt\n";
                cmd += "set title 'Wyinterpolowany wielomian'\n";
                cmd += "set xlabel 'x'\n set ylabel 'y'\n set key inside left top\n";
                cmd += "plot \"polynomial.dat\" title 'Wielomian' w l, \"function.dat\" title 'Funkcja z pliku' w l, \"filtered.dat\" title 'Przefiltrowane punkty' w p\n";
                display = "wielomian, funkcja z pliku i filtrowane punkty";
                break;
            }
            break;
        case 52:
            cls();
            cout<<"Wybierz algorytm filtracji znalezionych punktow:\n\n";
            cout<<"1   - Algorytm usredniania\n";
            cout<<"2   - Algorytm wybierania co n-tego punktu\n";
            cout<<"3   - Algorytm minimalnej roznicy wartosci\n";
            key = getch();
            switch(key)
            {
            case 49:
                algorithm = "usrednione punkty";
                algorithm_flag = 1;
                break;
            case 50:
                algorithm = "wybieranie co n-tego punktu";
                algorithm_flag = 2;
                break;
            case 51:
                algorithm = "minimalna roznica wartosci";
                algorithm_flag = 3;
                break;
            default:
                algorithm = "minimalna roznica wartosci";
                algorithm_flag = 3;
                break;
            }
            break;
        #ifdef _WIN32
        case 53:
            cls();
            cout<<"Podaj pelna sciezke pakietu gnuplot\n\n";
            cout<<"!!! UWAGA !!!\nSciezka nie moze zawierac zadnego znaku bialego.\n";
            cout<<"Do oddzielania kolejnych folderow nalezy uzywac slash ('/'):\n\n";
            cin>>directory;
            break;
        #endif // _WIN32
        case 104:
            cls();
            cout<<"Wybierz temat pomocy:\n\n";
            cout<<"1   - Algorytmy filtracji punktow\n";
            cout<<"2   - Przydatne informacje dotyczace uzywania programu\n";
            cout<<"3   - Prawa autorskie\n";
            cout<<"4   - Tworca programu\n";
            key = getch();
            switch(key)
            {
            case 49:
                cls();
                cout<<"--- Algorytm usredniania ---\n\n";
                cout<<"Filtruje punkty w taki sposob, ze pobiera pewna ilosc punktow zalezna od modyfikowalnego parametru \"gestosc\" i dokonuje ich sredniej arytmetycznej.\n\n";

                cout<<"--- Algorytm wybierania co n-tego punktu ---\n\n";
                cout<<"Filtruje punkty w taki sposob, ze wybiera tylko co n-ty punkt, ze stalym krokiem zalezym od modyfikowalnego parametru \"gestosc\".\n\n";

                cout<<"--- Algorytm minimalnej roznicy wartosci ---\n\n";

                cout<<"Filtruje punkty w taki sposob, ze sprawdza roznice wartosci funkcji w obecnym punkcie i nastepnym. Jezeli bezwzgledna ";
                cout<<"wartosc roznicy tych wartosci jest mniejsza od pewnej zadanej wielkosci zaleznej od modyfikowalnego parametru ";
                cout<<"\"gestosc\", algorytm szuka najblizszego punktu dla ktorego bezwzgledna wartosc roznicy tego punktu z punktem startowym ";
                cout<<"jest wieksza od zadanej. Jesli jednak takiego punktu dlugo nie da sie znalezc to algorytm dodaje automatycznie punkt ";
                cout<<"bez wzgledu na to czy spelnia powyzszy warunek. O tym kiedy algorytm automatycznie dodaje punkt rowniez decyduje ";
                cout<<"parametr zalezny od modyfikowalnego parametru \"gestosc\".";

                getch();
                break;
            case 50:
                cls();
                cout<<"--- Zalozenia dla pliku bitmap i narysowanej funkcji ---\n\n";
                cout<<"1. Obraz musi miec biale tlo\n\n";
                cout<<"2. Os x musi byc wyrazna prosta linia, przebiegajaca przez przynajmniej 80% szerokosci obrazu\n\n";
                cout<<"3. Grubosc lini bedacej osia musi wynosic dokladnie 1 pixel\n\n";
                cout<<"4. Wartosci funkcji sa okreslane na podstawie odleglosci danego punktu od osi x, w pixelach\n\n";
                cout<<"5. Do narysowania osi oraz funkcji mozna uzyc dowolnego koloru, oczywiscie po za bialym\n\n";
                cout<<"6. Nalezy miec swiadomosc ze niektore funkcje sa bardzo trudne do interpolacji i rezultat moze nie byc zadowalajacy\n\n";
                cout<<"7. Jezeli rezultat interpolacji nie jest zadowalajacy, nalezy sprobowac zmienic algorytm i/lub gestosc filtracji\n\n";
                cout<<"8. Jezeli interpolujemy funkcje dyskretna powinnismy przede wszystkim zwiekszyc gestosc filtracji\n\n";
                cout<<"9. Czesto zdarza sie tak, ze wynikowy wielomian interpoluje funkcje zadowalajaco, ale tylko w pewnym obszarze";

                getch();
                break;
            case 51:
                cls();

                cout<<"Copyright (c) 2005, The EasyBMP Project (http://easybmp.sourceforge.net)\n";
                cout<<"All rights reserved.\n\n";

                cout<<"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n\n";

                   cout<<"1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\n";
                   cout<<"2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer ";
                   cout<<"in the documentation and/or other materials provided with the distribution.\n";
                   cout<<"3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.\n\n";

                cout<<"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, ";
                cout<<"THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. ";
                cout<<"IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ";
                cout<<"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ";
                cout<<"HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ";
                cout<<"ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";

                getch();
                break;
            case 52:
                cls();

                cout<<"Program powstal w ramach projektu koncowego na przedmiot\n";
                cout<<"Metody numeryczne i symulacja\n\n";
                cout<<"Data stworzenia ostatecznej wersji:\n";
                cout<<"20.01.2014\n\n";
                cout<<"Informacje o tworcy:\n";
                cout<<"Arkadiusz Chorian\nStudent Automatyki i Robotyki\nWydzial Informatyczny\nPolitechnika Poznanska";

                getch();
                break;
            default:
                cout<<"\nNie wybrano tematu pomocy\n";
                getch();
                break;
            }
            break;
        case 100:
            bitmap = new BMP;
            if ( !bitmap->ReadFromFile(filename.c_str()) )
            {
                cout<<"\nNie podales nazwy pliku bitmapowego lub wskazany plik nie istnieje\n\n";
                cout<<"Aby kontynuowac nacisnij ENTER\n";
                getch();
                break;
            }
            bitmap->ReadFromFile(filename.c_str());
            data = new DataFromBitmap(thickness);
            data->find_axis(bitmap);
            data->find_points(bitmap);
            switch (algorithm_flag)
            {
            case 1:
                data->set_output_points1();
                break;
            case 2:
                data->set_output_points2();
                break;
            case 3:
                data->set_output_points3();
                break;
            default:
                data->set_output_points1();
                break;
            }
            interpolation = new Polynomial(data->filtered_points,data->founded_points,data->filtered_points.size()-1);
            interpolation->generate_points();
            interpolation->save_to_file();
            interpolation->plot(cmd,directory);
            delete bitmap;
            delete data;
            delete interpolation;
            break;
        default:
            break;
        }
    }

    cls();
    cout<<"Dziekuje za skorzystanie z mojego programu.\n";
    cout<<"Arkadiusz Chorian\n";
    getch();

    return EXIT_SUCCESS;
}
