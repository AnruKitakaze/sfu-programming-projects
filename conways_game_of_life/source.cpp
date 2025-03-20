#include <iostream>
#include <ctime>
#include <Windows.h>

#define CELL_SYMBOL 'Î' //Символ, выводимый на месте живой клетки.
#define INDENT      36 //Отступ в 3 строки для вывода информации
using namespace std;

//Ширина и высота на 2 ед. больше заданных пользователем
struct Settings {
	int         fieldWidth;
	int         fieldHeight;
	float       delay;
	long long   maxStep;
};

//Для получения сведений о настройках текущей игровой сессии
void primarySetup(Settings&);
//Вывод поля на экран консоли(WinAPI)
void printField(char**, const Settings&, HBRUSH&, HBRUSH&,
	const unsigned int&, HDC&);
//Заполнение поля пользователем
void fillField(char**, const Settings&);
//Заполнение поля случайным образом
void randomFillField(char**, const Settings&);
//Функция, занимающаяся организацией игрового процесса
void startGame(char**, const Settings&);
//Генерация следующего поколения клеток. Возвращает кол-во живых клеток
unsigned long long nextGenField(char**, const Settings&);
//Функция подсчёта живых соседей
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
		deletefield[i];
	}
	delete field;
	delete settings;
}

void primarySetup(Settings& settings) {
	cout << "Введите ширину игрового поля: ";
	cin >> settings.fieldWidth;
	while (settings.fieldWidth <= 0) {
		cout << "Введите значение больше нуля: ";
		cin >> settings.fieldWidth;
	}
	settings.fieldWidth += 2; //Увеличим ширину поле на 2

	cout << "Введите высоту игрового поля: ";
	cin >> settings.fieldHeight;
	while (settings.fieldHeight <= 0) {
		cout << "Введите значение больше нуля: ";
		cin >> settings.fieldHeight;
	}
	settings.fieldHeight += 2; //Увеличим высоту поля на 2

	cout << "Введите максимальное число генераций новых поколений"
		"(0 для отмены ограничения): ";
	cin >> settings.maxStep;

	cout << "Введите задержку в секундах между отрисовкой поколений: ";
	cin >> settings.delay;
	if (settings.delay < 0) {
		settings.delay = 0.0;
	}
	settings.delay *= 1000;
}

void printField(char** field, const Settings& settings, HBRUSH& Cells,
	HBRUSH& Background, const unsigned int& cellSize, HDC& dc) {
	//Перебираем все элементы(Кроме края поля)
	for (int line = 1; line < (settings.fieldHeight - 1); ++line) {
		for (int column = 1; column < (settings.fieldWidth - 1); ++column) {
			//Если нашли живую клетку, то рисуем её
			if (field[line][column] == CELL_SYMBOL) {
				SelectObject(dc, Cells);
				Ellipse(dc, column * cellSize - cellSize,
					line * cellSize - cellSize + INDENT,
					column * cellSize, line * cellSize + INDENT);
			}
			//Иначе рисуем пустую клетку
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
	//Буферная строка для заполнения поля
	char* inputLine = new char[settings.fieldWidth - 1];

	cout << "Производите заполнение по одной строке за раз.\n"
		<< "Вам необходимо вводить строки вида \"010010\",\n"
		<< "где 0 - мёртвая клетка, а 1 - живая клетка.\n";
	cin.ignore();
	//На местах единиц в строке ставятся живые клетки
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
	char* answer = new char;

	cout << "Заполнить поле вручную?(Y/N): ";
	cin >> *answer;
	if ('Y' == *answer || 'y' == *answer) {
		fillField(field, settings);
	}
	else {
		randomFillField(field, settings);
	}

	delete answer;

	cout << "Введите размер клетки(Целое число больше 3. "
		"Рекомендуемый размер = 8): ";
	cin >> cellSize;

	HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleDisplayMode(hConsoleHandle, CONSOLE_FULLSCREEN_MODE, &coords);

	system("cls");
	SetConsoleTextAttribute(hConsoleHandle, 0x06);
	cout << "Размер поля: " << settings.fieldWidth - 2
		<< "x" << settings.fieldHeight - 2 << ", задержка: "
		<< settings.delay / 1000 << ", максимум итераций: "
		<< settings.maxStep << endl;
	cout << "Для закрытия программы используйте сочетание клавиш Alt + F4.\n";

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
		cout << "Кол-во живых клеток: " << liveCells << "    ";
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
	//Ищем места для зарождения и уничтожения клеток
	for (int line = 1; line < (settings.fieldHeight - 1); ++line) {
		for (int column = 1; column < (settings.fieldWidth - 1); ++column) {
			*cellMates = countNeighbours(field, line, column);
			//Мертва и 3 соседа - оживить
			if (' ' == field[line][column] && 3 == *cellMates) {
				field[line][column] = 'Ð';
			}
			//Жива и не 2 и не 3 соседа - убить
			else if (CELL_SYMBOL == field[line][column] &&
				(2 != *cellMates && 3 != *cellMates)) {
				field[line][column] = 'Ì';
			}
		}
	}
	//Выращиваем следующее поколение
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
