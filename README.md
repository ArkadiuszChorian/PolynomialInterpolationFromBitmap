# Polynomial Interpolation From Bitmap

Projekt "Interpolacja wielomianu z bitmap" dotyczy zagadnień z dziedziny metod numerycznych i symulacji. W ramach projektu został napisany program w języku C++, który pozwala na interpolację funkcji zawartej w pliku bitmap. W praktyce oznacza to, że danymi wejściowymi programu jest plik *.bmp ze szkicem funkcji, natomiast w rezultacie otrzymujemy współczynniki wielomianu interpolującego (przybliżającego) funkcję oraz wykres przedstawiający czyste dane z bitmapy w porównaniu z wykresem wielomianu interpolującego. Do prezentacji graficznej wykorzystano pakiet gnuplot. Do przetwarzania obrazów bitmapowych wykorzystano bibliotekę EasyBMP. Program działa zarówno pod systemem Windows, jak i Linux.

Data wykonania: 2013

# Przykład

![alt text](https://github.com/archer333/PolynomialInterpolationFromBitmap/blob/master/PolynomialInterpolationFromBitmap/InterpolacjaBMPscreenshot.PNG "Screenshot")

# Uruchomienie

Aby przetestować program konieczne jest posiadanie na dysku pakietu gnuplot. Archiwum z pakietem zostało dołączone do repozytorium. Pakiet należy wypakować w dowolne miejsce na dysku i w programie wskazać ścieżkę do pliku gnuplot.exe.

Do projektu załączony został również plik makefile, dzięki któremy można ponownie skompilować źródła.

W katalogu Example bitmaps umieściłem 7 testowych funkcji obrazujących działanie programu.

Zalecane ustawienia dla tych funkcji:
Algorytmy: Minimalna roznica wartosci(MRW), Usrednione punkty (UP)

| Numer testu   | Gęstość       | Algorytm |
| ------------- |:-------------:| --------:|
| 1	| 50 | MRW
| 2	| 50 | MRW
| 3	| 70 | MRW
| 4	| 50 | MRW
| 5	| 10 | UP
| 6	| 50 | MRW
| 7	| 30 | UP

Dla tych ustawień funkcje powinny zostać przybliżone bardzo zadowalająco.
