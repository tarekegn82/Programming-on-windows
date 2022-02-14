#include <windows.h>
#include "resource.h"
#include <string>
#include <stdlib.h>
#include <time.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include <commctrl.h>
#include <tlhelp32.h>


using namespace std;
std::string admin_name, deposit_amount, withdraw_amount, total;

#define MAX_ADMIN 3
#define MAX_TRANS_AMOUNT 1000
#define MAX_TRANS_PDAY 1
#define SEGMENTS 50
int num_admins = 3;

// declare handles
HANDLE ghSemaphore;
HANDLE aThread[MAX_ADMIN];

PAINTSTRUCT ps;
HDC hdc;
RECT rt;

LRESULT OnCommand(HWND hWnd, int iID, int iEvent, HWND hWndControl, bool& isHandled);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR  CALLBACK ViewTransactionRecordDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

DWORD ThreadID[MAX_ADMIN];

int counter = 0;
int moneyAmount = 0;
int depositAmount = 0;
int withdrawAmount = 0;

SYSTEMTIME lt;
BOOL bObjectFlag = TRUE;

BOOL bAdminaFlag = FALSE;
BOOL bAdminbFlag = FALSE;
BOOL bAdmincFlag = FALSE;

BOOL tableFlag = FALSE;

POINT ptDeposit[SEGMENTS];
POINT ptWithdraw[SEGMENTS];
int posDeposit = 0, posWithdraw = 0;

class Bank;
DWORD WINAPI workerThread(LPVOID lpParam);
void textPainter(string str, int x, int y);
void curvePoints(Bank* bank, int point);
void UpdateTransactionTable(HWND hWndList);


class Bank {
    char admin_name;
    int adminWithdraw = 0;
    int adminDeposit = 0;
    int pos = 0;
    HANDLE hThread;
    int prev = 0;
    int bankChecker;

public:
    int posAdminWithdraw = 0, posAdminDeposit = 0;
    POINT ptAdminDeposit[SEGMENTS];
    POINT ptAdminWithdraw[SEGMENTS];
    BOOL bAdminDepositFlag = FALSE, bAdminWithdrawFlag = FALSE;
    DWORD dwThreadID;
    Bank(char name) {
        this->admin_name = name;
        create_Thread();
    }

    Bank* getObject() {
        return this;
    }

    HANDLE getHandle() {
        return hThread;
    }

    POINT* getDepositPoints() {
        return ptAdminDeposit;
    }

    POINT* getWithdrawPoints() {
        return ptAdminWithdraw;
    }

    char getAdminName() {
        return admin_name;
    }

    SYSTEMTIME transaction_time() {

        GetLocalTime(&lt);
        return lt;
    }

    int deposit(int amount) {
        SYSTEMTIME t;

        bAdminDepositFlag = TRUE;
        bAdminWithdrawFlag = FALSE;
        moneyAmount += amount;
        adminDeposit += amount;
        depositAmount += amount;
        t = transaction_time();
        curvePoints(this, amount);

        return moneyAmount;
    }

    int withdraw(int amount) {
        SYSTEMTIME t;
        bAdminDepositFlag = FALSE;
        bAdminWithdrawFlag = TRUE;

        if (moneyAmount > amount) {
            moneyAmount -= amount;
            adminWithdraw += amount;
            withdrawAmount += amount;
        }
        else {
            amount = 0;
        }
        curvePoints(this, amount);
        t = transaction_time();
        return moneyAmount;
    }

    void performTransaction() {

        deposit((rand() % MAX_TRANS_AMOUNT));
        withdraw((rand() % MAX_TRANS_AMOUNT));

    }


    //create thread
    void create_Thread() {

        hThread = CreateThread(
            NULL,       // default security attributes
            0,          // default stack size
            (LPTHREAD_START_ROUTINE)workerThread,
            this,       // admin name
            0,          // default creation flags
            &dwThreadID); // receive thread identifier

        if (hThread == NULL)
        {
            exit(-1);
        }
    }

};


//create semaphore
HANDLE create_Semaphore() {
    HANDLE semaphore = CreateSemaphore(
        NULL,           // default security attributes
        1,  // initial count
        1,  // maximum count
        NULL);          // unnamed semaphore

    if (semaphore == NULL)
    {
        exit(-1);
    }
    return semaphore;
}


//close handles
void close_Handles() {

    for (int i = 0; i < num_admins; i++)
        CloseHandle(aThread[i]);

    CloseHandle(ghSemaphore);

}

void textPainter(string str, int x, int y) {

    ::SetTextColor(hdc, RGB(0, 0, 0));
    ::TextOut(hdc, x, y, str.c_str(), str.length());
}

void curvePoints(Bank* bank, int point) {
    if (bank->bAdminDepositFlag == TRUE)
    {
        (bank->ptAdminDeposit[bank->posAdminDeposit].y) = (SEGMENTS - point * SEGMENTS / MAX_TRANS_AMOUNT) / 2 + 2;
        (bank->ptAdminDeposit[bank->posAdminDeposit].x) = bank->posAdminDeposit * SEGMENTS / 6 + SEGMENTS / 9;
        bank->posAdminDeposit++;

        //total
        (ptDeposit[posDeposit].y) = (SEGMENTS - point * SEGMENTS / MAX_TRANS_AMOUNT) / 2 + 2;
        (ptDeposit[posDeposit].x) = posDeposit * SEGMENTS / 12 + SEGMENTS / 9;
        posDeposit++;
    }
    if (bank->bAdminWithdrawFlag == TRUE)
    {
        (bank->ptAdminWithdraw[bank->posAdminWithdraw].y) = (SEGMENTS - point * SEGMENTS / MAX_TRANS_AMOUNT) / 2 + 2;
        (bank->ptAdminWithdraw[bank->posAdminWithdraw].x) = bank->posAdminWithdraw * SEGMENTS / 6 + SEGMENTS / 9;
        bank->posAdminWithdraw++;

        //total
        (ptWithdraw[posWithdraw].y) = (SEGMENTS - point * SEGMENTS / MAX_TRANS_AMOUNT) / 2 + 2;
        (ptWithdraw[posWithdraw].x) = posWithdraw * SEGMENTS / 12 + SEGMENTS / 9;
        posWithdraw++;
    }

}

void curvePainter(Bank* bank) {
    HPEN hwPen = CreatePen(PS_SOLID, 1, RGB(150, 0, 0));
    HPEN hdPen = CreatePen(PS_SOLID, 1, RGB(0, 128, 0));
    HPEN hOldPen_1, hOldPen_2;
    
    if (bAdminaFlag && ('A' == bank->getAdminName()))
    {
        textPainter("ADMIN A", SEGMENTS / 3, 0.75 * SEGMENTS);
        hOldPen_1 = (HPEN)::SelectObject(hdc, hwPen);
        ::Polyline(hdc, bank->getWithdrawPoints(), MAX_TRANS_PDAY * 5);
        hOldPen_2 = (HPEN)::SelectObject(hdc, hdPen);
        ::Polyline(hdc, bank->getDepositPoints(), MAX_TRANS_PDAY * 5);
        textPainter("Withdraw", bank->getWithdrawPoints()[4].x, bank->getWithdrawPoints()[4].y);
        textPainter("Deposit", bank->getDepositPoints()[4].x, bank->getDepositPoints()[4].y);
    }
    if (bAdminbFlag && ('B' == bank->getAdminName()))
    {
        textPainter("ADMIN B", SEGMENTS / 3, 0.75 * SEGMENTS);
        hOldPen_1 = (HPEN)::SelectObject(hdc, hwPen);
        ::Polyline(hdc, bank->getWithdrawPoints(), MAX_TRANS_PDAY * 5);
        hOldPen_2 = (HPEN)::SelectObject(hdc, hdPen);
        ::Polyline(hdc, bank->getDepositPoints(), MAX_TRANS_PDAY * 5);
        textPainter("Withdraw", bank->getWithdrawPoints()[4].x, bank->getWithdrawPoints()[4].y);
        textPainter("Deposit", bank->getDepositPoints()[4].x, bank->getDepositPoints()[4].y);
    }
    if (bAdmincFlag && ('C' == bank->getAdminName()))
    {
        textPainter("ADMIN C", SEGMENTS / 3, 0.75 * SEGMENTS);
        hOldPen_1 = (HPEN)::SelectObject(hdc, hwPen);
        ::Polyline(hdc, bank->getWithdrawPoints(), MAX_TRANS_PDAY * 5);
        hOldPen_2 = (HPEN)::SelectObject(hdc, hdPen);
        ::Polyline(hdc, bank->getDepositPoints(), MAX_TRANS_PDAY * 5);
        textPainter("Withdraw", bank->getWithdrawPoints()[4].x, bank->getWithdrawPoints()[4].y);
        textPainter("Deposit", bank->getDepositPoints()[4].x, bank->getDepositPoints()[4].y);
    }
    if (!bAdminaFlag && !bAdminbFlag && !bAdmincFlag)
    {
        textPainter("TOTAL", SEGMENTS / 3, 0.75 * SEGMENTS);
        hOldPen_1 = (HPEN)::SelectObject(hdc, hwPen);
        ::Polyline(hdc, ptWithdraw, MAX_TRANS_PDAY * 5 * 2);
        hOldPen_2 = (HPEN)::SelectObject(hdc, hdPen);
        ::Polyline(hdc, ptDeposit, MAX_TRANS_PDAY * 5 * 2);
        textPainter("Withdraw", ptWithdraw[9].x, ptWithdraw[9].y);
        textPainter("Deposit", ptDeposit[9].x, ptDeposit[9].y);
    }
    ::DeleteObject(hwPen);
}

void UpdateTransactionTable(HWND hWndList)
{
    // Delete all items delete all items
    ::SendMessage(hWndList, LVM_DELETEALLITEMS, 0, 0);
    int nItem = 0; // Item number item count
    std::ifstream inFile("transaction.txt");
    if (inFile.is_open())
    {
        std::string line;
        while (std::getline(inFile, line))
        {
            std::stringstream ss(line);

            std::getline(ss, admin_name, ',');
            std::getline(ss, deposit_amount, ',');
            std::getline(ss, withdraw_amount, ',');
            std::getline(ss, total, ',');

            // Insert an item
            LVITEM item = { 0 };
            item.iItem = nItem;
            item.mask = LVIF_TEXT; // Determine the validity of pszText
            item.pszText = const_cast<char*>(admin_name.c_str()); // Set the texts
            ::SendMessage(hWndList, LVM_INSERTITEM, 0, (long)&item);

            // Set text for the new item
            LVITEM lvi;
            lvi.iSubItem = 1; // Set the text to the first column specifies to set the text of the first column
            lvi.pszText = const_cast<char*>(deposit_amount.c_str()); // Set the texts
            ::SendMessage(hWndList, LVM_SETITEMTEXT, nItem, (LPARAM)&lvi);


            // Set text for the new item
            LVITEM lvi2;
            lvi2.iSubItem = 2; // Set the text to the first column specifies to set the text of the first column
            lvi2.pszText = const_cast<char*>(withdraw_amount.c_str()); // Set the texts
            ::SendMessage(hWndList, LVM_SETITEMTEXT, nItem, (LPARAM)&lvi2);

            // Set text for the new item
            LVITEM lvi3;
            lvi3.iSubItem = 3; // Set the text to the first column specifies to set the text of the first column
            lvi3.pszText = const_cast<char*>(total.c_str()); // Set the texts
            ::SendMessage(hWndList, LVM_SETITEMTEXT, nItem, (LPARAM)&lvi3);
            nItem++;
        }
    }
}

void weekdayName(int day) {
    string weekday;
    int x = 200;
    switch (day + 1)
    {
    case 1:
        weekday = "MONDAY";
        break;
    case 2:
        weekday = "TUESDAY";
        break;
    case 3:
        weekday = "WEDNESDAY";
        break;
    case 4:
        weekday = "THURSDAY";
        break;
    case 5:
        weekday = "FRIDAY";
        break;
    default:
        weekday = "Weekend";
    }
    textPainter(weekday, (day)*SEGMENTS / 6 + SEGMENTS / 12, 1);
}


void startingPoint(void)
{

    //Create Semaphore
    ghSemaphore = create_Semaphore();


    // create a Bank object
    Bank adminA('A');
    Bank adminB('B');
    Bank adminC('C');


    aThread[0] = adminA.getHandle();
    aThread[1] = adminB.getHandle();
    aThread[2] = adminC.getHandle();

    ThreadID[0] = adminA.dwThreadID;
    ThreadID[1] = adminB.dwThreadID;
    ThreadID[2] = adminC.dwThreadID;
    // Wait for all threads to terminate
    WaitForMultipleObjects(num_admins, aThread, TRUE, INFINITE);


    close_Handles();
}

INT_PTR  CALLBACK ViewTransactionRecordDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        // Initialize List Control Initialize the list view control

        HWND hWndList = ::GetDlgItem(hdlg, IDC_TLIST);

        // Set the extended style
        ::SendMessage(hWndList, LVM_SETEXTENDEDLISTVIEWSTYLE,
            0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        LVCOLUMN column;
        // determine the validity of pszText, fmt, cx in structure LVCOLUMN
        column.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
        // Set the validity of attributes
        column.fmt = LVCFMT_CENTER; // Display text in center Specifies the text to be displayed in the center
        column.cx = 120; // Width of column specifies the width of this column
        std::string c1 = "Admin Name";
        column.pszText = const_cast<char*>(c1.c_str());// Texts to display Specify the text displayed in this column
        // Add a new column Add a new column
        ::SendMessage(hWndList, LVM_INSERTCOLUMN, 0, (LPARAM)&column);

        // Add another column for name
        std::string c2 = "Deposit Amount";
        column.pszText = const_cast<char*>(c2.c_str());
        column.cx = 125;
        ::SendMessage(hWndList, LVM_INSERTCOLUMN, 1, (LPARAM)&column);

        // Add another column for amount
        std::string c3 = "Withdraw Amount";
        column.pszText = const_cast<char*>(c3.c_str());
        column.cx = 125;
        ::SendMessage(hWndList, LVM_INSERTCOLUMN, 2, (LPARAM)&column);

        //Add another column for total
        std::string c4 = "Total";
        column.pszText = const_cast<char*>(c4.c_str());
        column.cx = 100;
        ::SendMessage(hWndList, LVM_INSERTCOLUMN, 3, (LPARAM)&column);

        // Update process list refresh process list
        UpdateTransactionTable(hWndList);
    }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDTCANCEL:
            ::EndDialog(hdlg, IDTCANCEL);
            break;
        }
        break;
    }
    return 0;
}

DWORD usingCriticalSection(Bank* bank) {
    DWORD dwWaitResult;
    BOOL bContinue = TRUE;
    srand(time(0));
    GetLocalTime(&lt);


    while (bContinue)
    {
        for (int i = 0; i < (5); i++)
            for (int j = 0; j < MAX_TRANS_PDAY; j++)
            {
                dwWaitResult = WaitForSingleObject(
                    ghSemaphore,    // handle to semaphore
                    INFINITE);  // infinite time-out interval

                switch (dwWaitResult)
                {
                    // The semaphore object was signaled.
                case WAIT_OBJECT_0:

                    bContinue = FALSE;
                    bank->performTransaction();

                    if (!ReleaseSemaphore(
                        ghSemaphore,  // handle to semaphore
                        1,            // increase count by one
                        NULL))       // not interested in previous count
                    {
                        textPainter("ReleaseSemaphore error", SEGMENTS / 10, SEGMENTS / 10);
                        //printf("ReleaseSemaphore error: %d\n", GetLastError());
                    }

                    break;

                case WAIT_ABANDONED:
                    return FALSE;
                }
            }
    }

    curvePainter(bank);
    return TRUE;

}

DWORD WINAPI workerThread(LPVOID lpParam)
{
    DWORD dwWaitResult;
    Bank* bank = static_cast<Bank*>(lpParam);

    return usingCriticalSection(bank);
}

// WndProc Prototype
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{

    char szClassName[] = "MainWClass";
    WNDCLASSEX wndclass;
    // WNDCLASSEX structure setting
    wndclass.cbSize = sizeof(wndclass); // Structure Size
    wndclass.style = CS_HREDRAW | CS_VREDRAW; // Redraw if size changed
    wndclass.lpfnWndProc = MainWndProc; // pointer to WndProc function
    wndclass.cbClsExtra = 0; // no extra class memory
    wndclass.cbWndExtra = 0; // no extra window memory
    wndclass.hInstance = hInstance; // Instance handle
    wndclass.hIcon = ::LoadIcon(hInstance,
        MAKEINTRESOURCE(IDI_ICON1)); // user defined icon
    wndclass.hCursor = ::LoadCursor(NULL,
        IDC_ARROW); // predefined cursor
    wndclass.hbrBackground =
        (HBRUSH)(COLOR_3DFACE + 1); // Predefined brush
    wndclass.lpszMenuName = (LPSTR)IDR_MENU1;
    wndclass.lpszClassName = szClassName; // class name
    wndclass.hIconSm = NULL; // no small icon of the class
    // register the window
    ::RegisterClassEx(&wndclass);
    // create the window
    HWND hwnd = ::CreateWindowEx(
        WS_EX_CLIENTEDGE, // dwExStyle?extended style
        szClassName, // lpClassName
        "Bank", // lpWindowName?title
        WS_OVERLAPPEDWINDOW, // dwStyle?style of window
        CW_USEDEFAULT, // X?initial X
        CW_USEDEFAULT, // Y?initial Y
        CW_USEDEFAULT, // nWidth
        CW_USEDEFAULT, // nHeight
        NULL, // hWndParent
        NULL, // hMenu
        hInstance, // hlnstance
        NULL); // lpParam?user data
    if (hwnd == NULL)
    {
        ::MessageBox(NULL, "Error in Creation?", "error", MB_OK);
        return -1;
    }


    // show window, update window
    ::ShowWindow(hwnd, nCmdShow);
    ::UpdateWindow(hwnd);
    // get message from message queue


    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0))
    {
        // translate keyboard message
        ::TranslateMessage(&msg);
        // dispatch message to WndProc
        ::DispatchMessage(&msg);
    }
    // when GetMessage returns 0, the program ends
    return msg.wParam;
}

LRESULT OnCommand(HWND hWnd, int iID, int iEvent, HWND hWndControl, bool& isHandled)
{

 
    switch (iID)
    {
    case ID_TRANSACTION_TABLE:
        tableFlag = TRUE;
        DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ViewTransactionRecord), hWnd, ViewTransactionRecordDlgProc);
        break;
    }

    return 0;
}


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // str used to buffer the string in the client area.
    // to use string class?we need??include <string>?
    static std::string str;

    int cX = SEGMENTS / 2, cY = SEGMENTS / 2;
    switch (message)
    {
    case WM_CREATE:
    {
        // setting the title
        ::SetWindowText(hwnd, "Bank");

        //::InvalidateRect(hwnd, NULL, 0);
        //::SendMessage(hwnd, WM_SETTEXT, 0, (long) "The simplest program of using resource");
        return 0;
    }
    case WM_COMMAND: // wParam (LOWORD) the lower bits contain the menu ID
        switch (LOWORD(wParam))
        {
        case ID_BANK_EXIT:
            ::SendMessage(hwnd, WM_CLOSE, 0, 0);
            break;
        case ID_ADMINS_ADMINA:
            bAdminaFlag = TRUE;
            ::InvalidateRect(hwnd, NULL, 1);
            break;
        case ID_ADMINS_ADMINB:
            bAdminbFlag = TRUE;
            ::InvalidateRect(hwnd, NULL, 1);
            break;
        case ID_ADMINS_ADMINC:
            bAdmincFlag = TRUE;
            ::InvalidateRect(hwnd, NULL, 1);
            break;
        case ID_DRAWING_CURVE:
            tableFlag = FALSE;
            ::InvalidateRect(hwnd, NULL, 1);
            break;
        case ID_TRANSACTION_TABLE:
            tableFlag = TRUE;
            //::InvalidateRect(hwnd, NULL, 1);
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(ViewTransactionRecord), hwnd, ViewTransactionRecordDlgProc);
            break;
        }
    case WM_PAINT:
    {

        hdc = ::BeginPaint(hwnd, &ps);

        // Get the size of client area       
        GetClientRect(hwnd, &rt);
        int cx = rt.right;
        int cy = rt.bottom;
        POINT pt[SEGMENTS];
        int point;

        HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
        SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
        ::SetMapMode(hdc, MM_ANISOTROPIC);
        ::SetWindowExtEx(hdc, SEGMENTS, SEGMENTS, NULL);
        ::SetViewportExtEx(hdc, cx, cy, NULL);
        ::SetViewportOrgEx(hdc, 0, 0, NULL);

        
        if (!tableFlag) {
            for (int i = 0; i < 5; i++)
                weekdayName(i);
            MoveToEx(hdc, SEGMENTS / 12, SEGMENTS / 2 + 2, (LPPOINT)NULL);
            LineTo(hdc, SEGMENTS, SEGMENTS / 2 + 2);
            MoveToEx(hdc, SEGMENTS / 12, SEGMENTS / 12, (LPPOINT)NULL);
            LineTo(hdc, SEGMENTS / 12, SEGMENTS / 2 + 2);
            startingPoint();
        }
        
        ::DeleteObject(::SelectObject(hdc, brush));
        ::EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_CHAR:
    {
        // buffering the ansi code
        str = str + char(wParam);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        tableFlag = TRUE;
        ::InvalidateRect(hwnd, NULL, 1);
        return 0;
    }
    case WM_DESTROY: // Destroy the window
    // Sending a WM_QUIT message queue?enforcing GetMessage() returns 0?ending the message loop
        ::PostQuitMessage(0);
        return 0;
    }
    // Send the messages that we cannot process to the system for default processing
    return ::DefWindowProc(hwnd, message, wParam, lParam);
}
