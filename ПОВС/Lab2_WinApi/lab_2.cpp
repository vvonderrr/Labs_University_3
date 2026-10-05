#define  STRICT
#define  WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <Windows.h>

// Число прямоугольников: 4, 6 или 9.
// #define - не переменная, а текстовая подстановка препроцессора.
#define CELL_COUNT 4

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI  WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE, // hPrevInstance,
	_In_ LPSTR, // lpCmdLine,
	_In_ int nCmdShow
)
{

	LPCTSTR szClass = TEXT("WinApiLab2WindowClass");

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = szClass;

	::RegisterClass(&wc);


	// Create Main window
	HWND hWnd = ::CreateWindow(szClass, "Test Mouse", WS_OVERLAPPEDWINDOW,
		0, 0, 400, 200, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) {
		return -1;
	}

	::ShowWindow(hWnd, nCmdShow);
	// ::UpdateWindow(hWnd);

	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0)) { // idle
		::DispatchMessage(&msg);
	}

	return 0;
}
//=========================================================

// Сколько столбцов и строк: 4 -> 2x2, 6 -> 3x2, 9 -> 3x3
void GetGrid(int* pCols, int* pRows)
{
	*pCols = (CELL_COUNT == 4) ? 2 : 3;
	*pRows = CELL_COUNT / *pCols;
}
//=========================================================

// Записывает в *prc прямоугольник клетки номер index.
// Для index == -1 ("ничего не подсвечено") - пустой прямоугольник.
void GetCellRect(HWND hWnd, int index, RECT* prc)
{
	if (index < 0) { ::SetRectEmpty(prc); return; }

	RECT rcClient;
	::GetClientRect(hWnd, &rcClient);

	int cols, rows;
	GetGrid(&cols, &rows);

	int w = rcClient.right / cols;   // ширина клетки
	int h = rcClient.bottom / rows;   // высота клетки
	int col = index % cols;             // номер столбца
	int row = index / cols;             // номер строки

	prc->left = col * w;
	prc->top = row * h;
	// Крайние клетки дотягиваются до края окна, чтобы не осталось
	// пикселей, "потерянных" при целочисленном делении.
	prc->right = (col == cols - 1) ? rcClient.right : prc->left + w;
	prc->bottom = (row == rows - 1) ? rcClient.bottom : prc->top + h;
}
//=========================================================

// Номер клетки, в которой лежит точка pt, или -1
int HitTestCell(HWND hWnd, POINT pt)
{
	for (int i = 0; i < CELL_COUNT; i++) {
		RECT rc;
		GetCellRect(hWnd, i, &rc);
		if (::PtInRect(&rc, pt)) return i;
	}
	return -1;
}
//=========================================================

// Если подсвеченная клетка изменилась - запомнить новую и заказать перерисовку.
// Сама функция НИЧЕГО не рисует.
void SetHovered(HWND hWnd, int newIndex)
{
	int oldIndex = (int)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

	RECT rcOld, rcNew;
	GetCellRect(hWnd, oldIndex, &rcOld);
	GetCellRect(hWnd, newIndex, &rcNew);

	if (!::EqualRect(&rcOld, &rcNew)) {
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)newIndex);
		::InvalidateRect(hWnd, NULL, FALSE); // FALSE: фон не стирать, WM_PAINT зальёт всё сам
	}
}
//=========================================================

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_CREATE:
		// Изначально "карман" окна содержит 0 = номер первой клетки.
		// Явно записываем -1: ничего не подсвечено.
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)-1);
		return 0;

	case WM_SIZE:
		// Размер изменился -> сетка сдвинулась -> перерисовать всё окно
		::InvalidateRect(hWnd, NULL, FALSE);
		return 0;

	case WM_MOUSEMOVE: {
		// x - младшие 16 бит lParam, y - старшие; short - чтобы сохранить знак
		POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
		SetHovered(hWnd, HitTestCell(hWnd, pt));
		return 0;
	}

	case WM_NCMOUSEMOVE:
		// Курсор на заголовке/рамке - снимаем подсветку
		SetHovered(hWnd, -1);
		break; // не return: дальше сообщение обработает DefWindowProc

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hDC = ::BeginPaint(hWnd, &ps);

		int hovered = (int)::GetWindowLongPtr(hWnd, GWLP_USERDATA);

		// 1. Заливка ВСЕХ клеток: в классе окна нет кисти фона,
		//    поэтому белый фон тоже рисуем сами
		for (int i = 0; i < CELL_COUNT; i++) {
			RECT rc;
			GetCellRect(hWnd, i, &rc);
			HBRUSH hBrsh = ::CreateSolidBrush(
				(i == hovered) ? RGB(220, 220, 255) : RGB(255, 255, 255));
			::FillRect(hDC, &rc, hBrsh);
			::DeleteObject(hBrsh);
		}

		// 2. Линии - поверх заливки
		RECT rcClient;
		::GetClientRect(hWnd, &rcClient);
		int cols, rows;
		GetGrid(&cols, &rows);

		HPEN hPen = ::CreatePen(PS_SOLID, 2, RGB(160, 160, 160));
		HPEN oPen = (HPEN)::SelectObject(hDC, hPen);

		for (int i = 1; i < cols; i++) {        // вертикальные линии
			int x = i * (rcClient.right / cols);
			::MoveToEx(hDC, x, 0, NULL);
			::LineTo(hDC, x, rcClient.bottom);
		}
		for (int i = 1; i < rows; i++) {        // горизонтальные линии
			int y = i * (rcClient.bottom / rows);
			::MoveToEx(hDC, 0, y, NULL);
			::LineTo(hDC, rcClient.right, y);
		}

		::SelectObject(hDC, oPen); // вернуть старое перо
		::DeleteObject(hPen);      // и только потом удалить своё

		::EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	return ::DefWindowProc(hWnd, message, wParam, lParam);
}
//=========================================================