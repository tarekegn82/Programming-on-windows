#include "afxwin.h"
#include <windows.h>
#include "resource.h"
#include <string>
#include <stdlib.h>
#include <time.h>
#include "LoginDlg.h"

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
#define SEGMENTS 100 
#define SEGMENTS 100 
int num_admins = 3;

// declare handles                                
HANDLE ghSemaphore;
HANDLE aThread[MAX_ADMIN];

PAINTSTRUCT ps;
HDC hdc;
RECT rt;
RECT m_rcInfo;


DWORD ThreadID[MAX_ADMIN];

int counter = 0;
int moneyAmount = 0;
int depositAmount = 0;
int withdrawAmount = 0;
int options = 1;
int n = 0;

int deposit_ad[3][5];
int withdraw_ad[3][5];

int count_no1 = 0;
int count_no2 = 0;
int recieve_position = 0;
int send_position = 0;

SYSTEMTIME lt;
BOOL bObjectFlag = TRUE;

BOOL bAdminaFlag = FALSE;
BOOL bAdminbFlag = FALSE;
BOOL bAdmincFlag = FALSE;
BOOL tableFlag = FALSE;
BOOL curveFlag = FALSE;
int checker = 10;

POINT ptDeposit[SEGMENTS];
POINT ptWithdraw[SEGMENTS];
int posDeposit = 0, posWithdraw = 0;

class Bank;
DWORD WINAPI workerThread(LPVOID lpParam);
void textPainter(string str, int x, int y);
void curvePoints(Bank* bank, int point);
void textPainter(string str, int x, int y); //////////////////////////////////////////////////////////////////////////////////////////////////////////

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
        if (count_no1 >= 5) count_no1 = 0;

        if (this->admin_name == 'A') {
            deposit_ad[0][count_no1] += amount;
        }
        else if (this->admin_name == 'B') {
            deposit_ad[1][count_no1] += amount;
        }
        else {
            deposit_ad[2][count_no1] += amount;
        }

        moneyAmount += amount;
        adminDeposit += amount;
        depositAmount += amount;
        count_no1++;
        t = transaction_time();
        curvePoints(this, amount);

        return moneyAmount;
    }

    int withdraw(int amount) {
        SYSTEMTIME t;
        bAdminDepositFlag = FALSE;
        bAdminWithdrawFlag = TRUE;

        if (moneyAmount > amount) {
            if (count_no1 >= 5)count_no1 = 0;
            if (this->admin_name == 'A') {
                withdraw_ad[0][count_no1] += amount;
            }
            else if (this->admin_name == 'B') {
                withdraw_ad[1][count_no1] += amount;
            }
            else {
                withdraw_ad[2][count_no1] += amount;
            }
            moneyAmount -= amount;
            adminWithdraw += amount;
            withdrawAmount += amount;
            count_no1++;
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

/////////////////////////////////////////////////////////
void Draw_Table() {

    if (n == 0) {
        textPainter("Admin A", SEGMENTS / 10 + 20, SEGMENTS / 10 + 70);
    }
    else if (n == 1) {
        textPainter("Admin B", SEGMENTS / 10 + 20, SEGMENTS / 10 + 70);
    }
    else {
        textPainter("Admin C", SEGMENTS / 10 + 20, SEGMENTS / 10 + 70);
    }

    textPainter("Day ", SEGMENTS / 10 + 5, SEGMENTS / 10 + 3);
    textPainter("Deposit", SEGMENTS / 10 + 20, SEGMENTS / 10 + 3);
    textPainter("Withdraw", SEGMENTS / 10 + 35, SEGMENTS / 10 + 3);

    textPainter("Monday", SEGMENTS / 10 + 5, SEGMENTS / 10 + 13);
    textPainter(to_string(withdraw_ad[n][0]), SEGMENTS / 10 + 20, SEGMENTS / 10 + 13);
    textPainter(to_string(deposit_ad[n][0]), SEGMENTS / 10 + 35, SEGMENTS / 10 + 13);

    textPainter("Tuesday", SEGMENTS / 10 + 5, SEGMENTS / 10 + 23);
    textPainter(to_string(withdraw_ad[n][1]), SEGMENTS / 10 + 20, SEGMENTS / 10 + 23);
    textPainter(to_string(deposit_ad[n][1]), SEGMENTS / 10 + 35, SEGMENTS / 10 + 23);

    textPainter("Wednesday", SEGMENTS / 10 + 5, SEGMENTS / 10 + 33);
    textPainter(to_string(withdraw_ad[n][2]), SEGMENTS / 10 + 20, SEGMENTS / 10 + 33);
    textPainter(to_string(deposit_ad[n][2]), SEGMENTS / 10 + 35, SEGMENTS / 10 + 33);

    textPainter("Thursday", SEGMENTS / 10 + 5, SEGMENTS / 10 + 43);
    textPainter(to_string(withdraw_ad[n][3]), SEGMENTS / 10 + 20, SEGMENTS / 10 + 43);
    textPainter(to_string(deposit_ad[n][3]), SEGMENTS / 10 + 35, SEGMENTS / 10 + 43);

    textPainter("Friday", SEGMENTS / 10 + 5, SEGMENTS / 10 + 53);
    textPainter(to_string(withdraw_ad[n][4]), SEGMENTS / 10 + 20, SEGMENTS / 10 + 53);
    textPainter(to_string(deposit_ad[n][4]), SEGMENTS / 10 + 35, SEGMENTS / 10 + 53);

    MoveToEx(hdc, SEGMENTS / 10, SEGMENTS / 10, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 2 + 5, SEGMENTS / 10);
    MoveToEx(hdc, SEGMENTS / 10, SEGMENTS / 5, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 2 + 5, SEGMENTS / 5);
    MoveToEx(hdc, SEGMENTS / 10, SEGMENTS / 5 + 10, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 2 + 5, SEGMENTS / 5 + 10);
    MoveToEx(hdc, SEGMENTS / 10, SEGMENTS / 5 + 20, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 2 + 5, SEGMENTS / 5 + 20);
    MoveToEx(hdc, SEGMENTS / 10, SEGMENTS / 5 + 30, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 2 + 5, SEGMENTS / 5 + 30);
    MoveToEx(hdc, SEGMENTS / 10, SEGMENTS / 5 + 40, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 2 + 5, SEGMENTS / 5 + 40);
    MoveToEx(hdc, SEGMENTS / 10, SEGMENTS / 5 + 50, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 2 + 5, SEGMENTS / 5 + 50);

    MoveToEx(hdc, SEGMENTS / 10, SEGMENTS / 10, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 10, SEGMENTS / 5 + 50);
    MoveToEx(hdc, SEGMENTS / 10 + 15, SEGMENTS / 10, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 10 + 15, SEGMENTS / 5 + 50);
    MoveToEx(hdc, SEGMENTS / 10 + 30, SEGMENTS / 10, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 10 + 30, SEGMENTS / 5 + 50);
    MoveToEx(hdc, SEGMENTS / 10 + 45, SEGMENTS / 10, (LPPOINT)NULL);
    LineTo(hdc, SEGMENTS / 10 + 45, SEGMENTS / 5 + 50);
}

//////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////

class CBankApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
};

class CMainWindow : public CWnd
{
public:
    CMainWindow();
protected:
    char m_szText[1024];	// buffer          
    CMenu menu1;

protected:
    virtual void PostNcDestroy();
    afx_msg BOOL OnCreate(LPCREATESTRUCT);
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnExitCommand();
    afx_msg void OnAdminACommand();
    afx_msg void OnAdminBCommand();
    afx_msg void OnAdminCCommand();
    afx_msg void OnCurve();
    afx_msg void OnDrawTable();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    DECLARE_MESSAGE_MAP()
};

CBankApp bankApp;

BOOL CBankApp::InitInstance()
{

    LoginDlg loginD;
    loginD.DoModal();
    m_pMainWnd = new CMainWindow;
    //bAdminaFlag = loginD.bAdminaFlag;
    //bAdminbFlag = loginD.bAdminbFlag;
    ::ShowWindow(*m_pMainWnd, m_nCmdShow);
    ::UpdateWindow(*m_pMainWnd);
    return TRUE;
}

CMainWindow::CMainWindow()
{
    m_szText[0] = '\0';


    LPCTSTR lpszClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW), (HBRUSH)(COLOR_3DFACE + 1), AfxGetApp()->LoadIcon(IDI_ICON1));

    CreateEx(WS_EX_CLIENTEDGE, lpszClassName,
        "Bank", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL);

    menu1.LoadMenu(IDR_MENU1);
    SetMenu(&menu1);
}

// CMainWindow Table of message mapping
BEGIN_MESSAGE_MAP(CMainWindow, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_TIMER()
    ON_COMMAND(ID_BANK_EXIT, &CMainWindow::OnExitCommand)
    ON_COMMAND(ID_ADMINS_ADMINA, &CMainWindow::OnAdminACommand)
    ON_COMMAND(ID_ADMINS_ADMINB, &CMainWindow::OnAdminBCommand)
    ON_COMMAND(ID_ADMINS_ADMINC, &CMainWindow::OnAdminCCommand)
    ON_COMMAND(ID_DRAWING_CURVE, &CMainWindow::OnCurve)
    ON_COMMAND(ID_DRAWING_TABLE, &CMainWindow::OnDrawTable)

END_MESSAGE_MAP()

BOOL CMainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

    // Install the timer
    ::SetTimer(m_hWnd, IDT_TIMER, 1000, NULL);

    // dislpy the window on the top layer
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE);

    return TRUE;
}


void CMainWindow::OnLButtonDown(UINT nFlags, CPoint point) {
    tableFlag = TRUE;
    ::InvalidateRect(m_hWnd, &m_rcInfo, TRUE);
}

void CMainWindow::OnTimer(UINT nIDEvent)
{
    if (nIDEvent == IDT_TIMER)
    {

        ::InvalidateRect(m_hWnd, &m_rcInfo, TRUE);
    }
    posWithdraw = 0;
    posDeposit = 0;
}

void CMainWindow::OnPaint()
{
    hdc = ::BeginPaint(m_hWnd, &ps);


    // Get the size of client area       
    ::GetClientRect(m_hWnd, &m_rcInfo);
    int cx = m_rcInfo.right;
    int cy = m_rcInfo.bottom;
    POINT pt[SEGMENTS];
    int point;

    HBRUSH brush = CreateSolidBrush(RGB(255, 255, 224));
    SetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)brush);
    ::SetMapMode(hdc, MM_ANISOTROPIC);
    ::SetWindowExtEx(hdc, SEGMENTS, SEGMENTS, NULL);
    ::SetViewportExtEx(hdc, cx, cy, NULL);
    ::SetViewportOrgEx(hdc, 0, 0, NULL);

    if (tableFlag) {
        Draw_Table();
    }
    else {
        for (int i = 0; i < 5; i++)
            weekdayName(i);
        MoveToEx(hdc, SEGMENTS / 12, SEGMENTS / 2 + 2, (LPPOINT)NULL);
        LineTo(hdc, SEGMENTS, SEGMENTS / 2 + 2);
        MoveToEx(hdc, SEGMENTS / 12, SEGMENTS / 12, (LPPOINT)NULL);
        LineTo(hdc, SEGMENTS / 12, SEGMENTS / 2 + 2);
        startingPoint();
    }
    DeleteObject(SelectObject(hdc, brush));
    ::EndPaint(m_hWnd, &ps);
}

void CMainWindow::OnExitCommand() {
    PostNcDestroy();
}
void CMainWindow::OnDrawTable() {
    tableFlag = TRUE;
    ::InvalidateRect(m_hWnd, NULL, 1);
}
void CMainWindow::OnCurve() {
    tableFlag = FALSE;
    ::InvalidateRect(m_hWnd, NULL, 1);
}

void CMainWindow::OnAdminACommand() {
    bAdminaFlag = TRUE;
    bAdminbFlag = FALSE;
    bAdmincFlag = FALSE;
    n = 0;
    ::InvalidateRect(m_hWnd, NULL, 1);
}

void CMainWindow::OnAdminBCommand() {

    bAdminbFlag = TRUE;
    bAdminaFlag = FALSE;
    bAdmincFlag = FALSE;
    n = 1;
    ::InvalidateRect(m_hWnd, NULL, 1);
}

void CMainWindow::OnAdminCCommand()
{
    bAdmincFlag = TRUE;
    bAdminaFlag = FALSE;
    bAdminbFlag = FALSE;
    n = 2;
    ::InvalidateRect(m_hWnd, NULL, 1);
}

void CMainWindow::PostNcDestroy()
{
    delete this;
}


