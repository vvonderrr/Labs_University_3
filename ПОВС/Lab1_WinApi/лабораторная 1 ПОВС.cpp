#define  STRICT
#define  WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <Windows.h>

HWND hWndMain = NULL;
HWND hWndPopup = NULL;
HWND hWndChild = NULL;

// Флаг для отслеживания состояния TOPMOST у Временного окна
bool g_isTopmost = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); //сигнатура главная оконная процедура

int WINAPI  WinMain( //точка входа
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE, // hPrevInstance,
    _In_ LPSTR, // lpCmdLine,
    _In_ int nCmdShow
)
{ 

    LPCTSTR szClass = TEXT("WinApiLabWindowC34567898765678lass");

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc; //указатель на функцию оконной процедуры
    wc.hInstance = hInstance;
    wc.lpszClassName = szClass;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //Установка стандартного белого/системного цвета фона окна
    wc.style = CS_DBLCLKS; // поддержку двойных кликов мышью для WM_LBUTTONDBLCLK

    if (!::RegisterClass(&wc)) { // регистрируем класс ОДИН раз и сразу проверяем
        return -1; // Завершаем программу, так как без шаблона окна создать нельзя
    }

    // Главное окно
    hWndMain = ::CreateWindow(szClass, "Main Window", WS_OVERLAPPEDWINDOW,
        100, 100, 600, 400, NULL, NULL, hInstance, NULL);
    if (hWndMain == NULL) {
        return -1;
    }

    ::ShowWindow(hWndMain, nCmdShow);

    // Временное окно: стиль WS_POPUP, владелец - главное окно
    hWndPopup = ::CreateWindow(szClass, "Popup Window", WS_POPUP | WS_OVERLAPPEDWINDOW,
        750, 100, 400, 300, hWndMain, NULL, hInstance, NULL);
    if (hWndPopup != NULL) {
        ::ShowWindow(hWndPopup, SW_SHOWNORMAL);
    }

    // Дочернее окно: родитель - главное окно
    hWndChild = ::CreateWindow(szClass, "Child Window", WS_OVERLAPPEDWINDOW | WS_CHILD | WS_VISIBLE, //делает окно видимым сразу при создании
        20, 20, 250, 150, hWndMain, NULL, hInstance, NULL);
    if (hWndChild == NULL) {
        return -1;
    }

    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0)) {
        ::DispatchMessage(&msg);
    }

    return 0;
}
//=========================================================

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

        // 1. Одиночный клик ЛКМ: делаем кликнутое окно новым родителем
    case WM_LBUTTONDOWN:
    {
        // Проверяем, что кликнули НЕ в само дочернее окно
        if (hWnd != hWndChild) {
            // Если текущее окно еще не является родителем
            if (::GetParent(hWndChild) != hWnd) {
                // Просто сменяем родителя, не меняя координаты и размеры
                ::SetParent(hWndChild, hWnd);
            }
        }
        break;
    }

    // 2. Двойной клик ЛКМ: переключаем стиль TOPMOST у Временного окна
    case WM_LBUTTONDBLCLK:
    {
        if (hWnd == hWndPopup) {
            g_isTopmost = !g_isTopmost; // Переключаем флаг (true / false)

            // Функция из задания: меняем стиль TOPMOST через SetWindowPos
            ::SetWindowPos(
                hWndPopup, g_isTopmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE
            );

            // Обновляем заголовок для наглядности
            if (g_isTopmost) {
                ::SetWindowText(hWndPopup, TEXT("Popup Window [TOPMOST]"));
            }
            else {
                ::SetWindowText(hWndPopup, TEXT("Popup Window"));
            }
        }
        break;
    }

    case WM_DESTROY:
        if (hWnd == hWndMain) { // уничтожается главное окно?
            ::PostQuitMessage(0);
        }
        return 0;
    }
    return ::DefWindowProc(hWnd, message, wParam, lParam);
}