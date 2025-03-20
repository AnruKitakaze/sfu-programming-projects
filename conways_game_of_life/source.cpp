#include <iostream>
#include <ctime>
#include <Windows.h>

#define CELL_SYMBOL 'Î' //Ñèìâîë, âûâîäèìûé íà ìåñòå æèâîé êëåòêè.
#define INDENT      36 //Îòñòóï â 3 ñòðîêè äëÿ âûâîäà èíôîðìàöèè
using namespace std;

//Øèðèíà è âûñîòà íà 2 åä. áîëüøå çàäàííûõ ïîëüçîâàòåëåì
struct Settings {
	int         fieldWidth;
	int         fieldHeight;
	float       delay;
	long long   maxStep;
};

//Äëÿ ïîëó÷åíèÿ ñâåäåíèé î íàñòðîéêàõ òåêóùåé èãðîâîé ñåññèè
void primarySetup(Settings&);
//Âûâîä ïîëÿ íà ýêðàí êîíñîëè(WinAPI)
void printField(char**, const Settings&, HBRUSH&, HBRUSH&,
	const unsigned int&, HDC&);
//Çàïîëíåíèå ïîëÿ ïîëüçîâàòåëåì
void fillField(char**, const Settings&);
//Çàïîëíåíèå ïîëÿ ñëó÷àéíûì îáðàçîì
void randomFillField(char**, const Settings&);
//Ôóíêöèÿ, çàíèìàþùàÿñÿ îðãàíèçàöèåé èãðîâîãî ïðîöåññà
void startGame(char**, const Settings&);
//Ãåíåðàöèÿ ñëåäóþùåãî ïîêîëåíèÿ êëåòîê. Âîçâðàùàåò êîë-âî æèâûõ êëåòîê
unsigned long long nextGenField(char**, const Settings&);
//Ôóíêöèÿ ïîäñ÷¸òà æèâûõ ñîñåäåé
unsigned short int countNeighbours(char**, const int& line, const int& column);

void main() {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	SetConsoleTitle("Conway's Game of Life");

	Settings* settings = new Settings;
	primarySetup(*settings);

	char** field = new char*[settings->fieldHeight];
	for (int i = 0; i < settings->fieldHeight; ++i) {
		field[i] = new char[settings->fieldWidth];
	}

	startGame(field, *settings);

	system("pause");
	for (int i = 0; i < settings->fieldHeight; ++i) {
		delete[] field[i];
	}
	delete field;
	delete settings;
}

void primarySetup(Settings& settings) {
	cout << "Ââåäèòå øèðèíó èãðîâîãî ïîëÿ: ";
	cin >> settings.fieldWidth;
	while (settings.fieldWidth <= 0) {
		cout << "Ââåäèòå çíà÷åíèå áîëüøå íóëÿ: ";
		cin >> settings.fieldWidth;
	}
	settings.fieldWidth += 2; //Óâåëè÷èì øèðèíó ïîëå íà 2

	cout << "Ââåäèòå âûñîòó èãðîâîãî ïîëÿ: ";
	cin >> settings.fieldHeight;
	while (settings.fieldHeight <= 0) {
		cout << "Ââåäèòå çíà÷åíèå áîëüøå íóëÿ: ";
		cin >> settings.fieldHeight;
	}
	settings.fieldHeight += 2; //Óâåëè÷èì âûñîòó ïîëÿ íà 2

	cout << "Ââåäèòå ìàêñèìàëüíîå ÷èñëî ãåíåðàöèé íîâûõ ïîêîëåíèé"
		"(0 äëÿ îòìåíû îãðàíè÷åíèÿ): ";
	cin >> settings.maxStep;

	cout << "Ââåäèòå çàäåðæêó â ñåêóíäàõ ìåæäó îòðèñîâêîé ïîêîëåíèé: ";
	cin >> settings.delay;
	if (settings.delay < 0) {
		settings.delay = 0.0;
	}
	settings.delay *= 1000;
}

void printField(char** field, const Settings& settings, HBRUSH& Cells,
	HBRUSH& Background, const unsigned int& cellSize, HDC& dc) {
	//Ïåðåáèðàåì âñå ýëåìåíòû(Êðîìå êðàÿ ïîëÿ)
	for (int line = 1; line < (settings.fieldHeight - 1); ++line) {
		for (int column = 1; column < (settings.fieldWidth - 1); ++column) {
			//Åñëè íàøëè æèâóþ êëåòêó, òî ðèñóåì å¸
			if (field[line][column] == CELL_SYMBOL) {
				SelectObject(dc, Cells);
				Ellipse(dc, column * cellSize - cellSize,
					line * cellSize - cellSize + INDENT,
					column * cellSize, line * cellSize + INDENT);
			}
			//Èíà÷å ðèñóåì ïóñòóþ êëåòêó
			else {
				SelectObject(dc, Background);
				Ellipse(dc, column * cellSize - cellSize,
					line * cellSize - cellSize + INDENT,
					column * cellSize, line * cellSize + INDENT);
			}
		}
	}
}

void fillField(char** field, const Settings& settings) {
	//Áóôåðíàÿ ñòðîêà äëÿ çàïîëíåíèÿ ïîëÿ
	char* inputLine = new char[settings.fieldWidth - 1];

	cout << "Ïðîèçâîäèòå çàïîëíåíèå ïî îäíîé ñòðîêå çà ðàç.\n"
		<< "Âàì íåîáõîäèìî ââîäèòü ñòðîêè âèäà \"010010\",\n"
		<< "ãäå 0 - ì¸ðòâàÿ êëåòêà, à 1 - æèâàÿ êëåòêà.\n";
	cin.ignore();
	//Íà ìåñòàõ åäèíèö â ñòðîêå ñòàâÿòñÿ æèâûå êëåòêè
	for (int line = 1; line < (settings.fieldHeight - 1); ++line) {
		cin.getline(inputLine, settings.fieldWidth - 1);
		for (int column = 1; column < (settings.fieldWidth - 1); ++column) {
			if ('1' == inputLine[column - 1]) {
				field[line][column] = CELL_SYMBOL;
			}
			else {
				field[line][column] = ' ';
			}
		}
	}
	delete inputLine;
}

void randomFillField(char** field, const Settings& settings) {
	srand(time(0));
	bool lifeCell = false;
	for (int i = 1; i < (settings.fieldHeight - 1); ++i) {
		for (int j = 1; j < (settings.fieldWidth - 1); ++j) {
			if (0 == (rand() % 2))
				lifeCell = true;
			else
				lifeCell = false;
			if (true == lifeCell) field[i][j] = CELL_SYMBOL;
			else field[i][j] = ' ';
		}
	}
}

void startGame(char** field, const Settings& settings) {
	unsigned long long  liveCells;
	unsigned short      cellSize;
	COORD               coords;
	char*               answer = new char;

	cout << "Çàïîëíèòü ïîëå âðó÷íóþ?(Y/N): ";
	cin >> *answer;
	if ('Y' == *answer || 'y' == *answer) {
		fillField(field, settings);
	}
	else {
		randomFillField(field, settings);
	}

	delete answer;

	cout << "Ââåäèòå ðàçìåð êëåòêè(Öåëîå ÷èñëî áîëüøå 3. "
		"Ðåêîìåíäóåìûé ðàçìåð = 8): ";
	cin >> cellSize;

	HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleDisplayMode(hConsoleHandle, CONSOLE_FULLSCREEN_MODE, &coords);

	system("cls");
	SetConsoleTextAttribute(hConsoleHandle, 0x06);
	cout << "Ðàçìåð ïîëÿ: " << settings.fieldWidth - 2
		<< "x" << settings.fieldHeight - 2 << ", çàäåðæêà: "
		<< settings.delay / 1000 << ", ìàêñèìóì èòåðàöèé: "
		<< settings.maxStep << endl;
	cout << "Äëÿ çàêðûòèÿ ïðîãðàììû èñïîëüçóéòå ñî÷åòàíèå êëàâèø Alt + F4.\n";

	HWND    hwnd = GetConsoleWindow();
	HDC     dc = GetDC(hwnd);
	HBRUSH  Cells;
	HBRUSH  Background;
	Cells = CreateSolidBrush(RGB(212, 156, 69));
	Background = CreateSolidBrush(RGB(30, 23, 5));

	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	GetConsoleScreenBufferInfo(hConsoleHandle, &consoleInfo);
	coords.X = 0;
	coords.Y = 2;

	printField(field, settings, Cells, Background, cellSize, dc);
	long long* currStep = new long long(0);
	do {
		Sleep(settings.delay);
		liveCells = nextGenField(field, settings);
		SetConsoleCursorPosition(hConsoleHandle, coords);
		cout << "Êîë-âî æèâûõ êëåòîê: " << liveCells << "    ";
		printField(field, settings, Cells, Background, cellSize, dc);
		++(*currStep);
	} while (settings.maxStep != *currStep && liveCells != 0);
	cout << endl;

	delete currStep;
	SetConsoleTextAttribute(hConsoleHandle, 0x07);
	ReleaseDC(hwnd, dc);
	DeleteObject(Cells);
	DeleteObject(Background);
}

unsigned long long nextGenField(char** field, const Settings& settings) {
	unsigned short int* cellMates = new unsigned short int;
	unsigned long long  count = 0;
	//Èùåì ìåñòà äëÿ çàðîæäåíèÿ è óíè÷òîæåíèÿ êëåòîê
	for (int line = 1; line < (settings.fieldHeight - 1); ++line) {
		for (int column = 1; column < (settings.fieldWidth - 1); ++column) {
			*cellMates = countNeighbours(field, line, column);
			//Ìåðòâà è 3 ñîñåäà - îæèâèòü
			if (' ' == field[line][column] && 3 == *cellMates) {
				field[line][column] = 'Ð';
			}
			//Æèâà è íå 2 è íå 3 ñîñåäà - óáèòü
			else if (CELL_SYMBOL == field[line][column] &&
				(2 != *cellMates && 3 != *cellMates)) {
				field[line][column] = 'Ì';
			}
		}
	}
	//Âûðàùèâàåì ñëåäóþùåå ïîêîëåíåå
	for (int line = 1; line < (settings.fieldHeight - 1); ++line) {
		for (int column = 1; column < (settings.fieldWidth - 1); ++column) {
			if (' ' != field[line][column]) {
				if ('Ì' == field[line][column]) {
					field[line][column] = ' ';
				}
				else {
					field[line][column] = CELL_SYMBOL;
					++count;
				}
			}
		}
	}
	delete cellMates;
	return count;
}
unsigned short int countNeighbours(char** field, const int& line,
	const int& column) {
	unsigned short int count = 0;
	for (int i = line - 1; i <= line + 1; ++i) {
		for (int j = column - 1; j <= column + 1; ++j) {
			if (CELL_SYMBOL == field[i][j]) ++count;
			else if ('Ì' == field[i][j]) ++count;
		}
	}
	if (CELL_SYMBOL == field[line][column]) --count;
	return count;
}
