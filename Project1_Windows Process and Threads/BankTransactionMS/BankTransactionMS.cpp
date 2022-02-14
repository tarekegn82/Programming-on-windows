/*Windows Process and Threads: Experimental Project*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>

using namespace std;

//define maximum number of admins
#define MAX_ADMIN 3
#define MAX_TRANS_PDAY 3
int num_admins = 3;
// declare handles
HANDLE ghSemaphore;
HANDLE ghMutex;
HANDLE aThread[MAX_ADMIN];
CRITICAL_SECTION CriticalSection;

DWORD ThreadID[MAX_ADMIN];
int counter = 0;
int moneyAmount = 0;
int depositAmount = 0;
int withdrawAmount = 0;
SYSTEMTIME lt;
BOOL displayDay = TRUE;

DWORD WINAPI workerThread(LPVOID lpParam);

class Bank {
    char admin_name;
    int adminWithdraw = 0;
    int adminDeposit = 0;

public:
    Bank(char name) {
        this->admin_name = name;
        create_Thread();
    }

    char getAdminName() {
        return admin_name;
    }

    string weekdayName(int day) {
        string weekday;
        GetLocalTime(&lt);
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
        return weekday;
    }

    SYSTEMTIME transaction_time() {

        GetLocalTime(&lt);
        return lt;
    }

    int deposit(int amount) {
        SYSTEMTIME t;
        printf("Admin %c: is Depositing money amount of %d\n", getAdminName(), amount);
        moneyAmount += amount;
        adminDeposit += amount;
        depositAmount += amount;
        t = transaction_time();
        printf("The Local Time is: %02d:%02d:%02d:%02d\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
        printf("Current Balance in this Account is %d\n\n", moneyAmount);
        return moneyAmount;
    }

    int withdraw(int amount) {
        SYSTEMTIME t;
        if (moneyAmount > amount) {
            printf("Admin %c: is withdrawing money amount of %d\n", getAdminName(), amount);
            moneyAmount -= amount;
            adminWithdraw += amount;
            withdrawAmount += amount;
        }

        else
            printf("Withdrawal by Admin %c unsuccessful! There is no enough balance\n", getAdminName());
        t = transaction_time();
        printf(" The Local Time is: %02d:%02d:%02d:%02d\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
        printf("Current Balance in this Account is %d\n\n", moneyAmount);
        return moneyAmount;
    }

    //Create Thread
    void create_Thread() {

        aThread[counter] = CreateThread(
            NULL,       // Default security attributes
            0,          // Default stack size
            (LPTHREAD_START_ROUTINE)workerThread,
            this,       // Admin name
            0,          // Default creation flags
            &ThreadID[counter]); // Receive thread identifier

        if (aThread[counter] == NULL)
        {
            printf("CreateThread Error: %d\n", GetLastError());
            exit(-1);
        }
        counter++;
    }

    void performTransaction() {

        deposit(rand());

        withdraw(rand());

    }

    void adminSummary() {
        printf("Admin: %c \n", getAdminName());
        printf("\tDeposit: %d\n", adminDeposit);
        printf("\tWithdraw: %d\n", adminWithdraw);
    }

};

//Create Semaphore
HANDLE create_Semaphore() {
    HANDLE semaphore = CreateSemaphore(
        NULL,           // default security attributes
        1,  // initial count
        1,  // maximum count
        NULL);          // unnamed semaphore

    if (semaphore == NULL)
    {
        printf("Create Semaphore Error: %d\n", GetLastError());
        exit(-1);
    }
    return semaphore;
}

//Create Mutex
HANDLE create_Mutex() {
    HANDLE mutex;
    mutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
    if (mutex == NULL)
    {
        printf("CreateMutex Error: %d\n", GetLastError());
        exit(-1);
    }

    return mutex;
}

//Close handles
void close_Handles() {

    for (int i = 0; i < num_admins; i++)
        CloseHandle(aThread[i]);

    CloseHandle(ghSemaphore);

    CloseHandle(ghMutex);
}

void totalSummary() {
    printf("=====This Week Transaction Summary=====\n");
    printf("General Transaction: \n");
    printf("\tDeposit: %d\n", depositAmount);
    printf("\tWithdraw: %d\n", withdrawAmount);
    printf("\tCurrent Balance: %d\n", moneyAmount);
}

int main(void)
{
    //Intialize critical section
    InitializeCriticalSection(&CriticalSection);

    //Create Semaphore
    ghSemaphore = create_Semaphore();

    //Create Mutex
    ghMutex = create_Mutex();

    // Create Bank objects
    Bank adminA('A');
    Bank adminB('B');
    Bank adminC('C');

    // Wait until all threads are terminated
    WaitForMultipleObjects(num_admins, aThread, TRUE, INFINITE);

    totalSummary();
    adminA.adminSummary();
    adminB.adminSummary();
    adminC.adminSummary();

    // Delete critical section
    DeleteCriticalSection(&CriticalSection);

    // Close thread, semaphore and mutex handles
    close_Handles();
    return 0;
}

DWORD usingCriticalSection(Bank* bank) {
    DWORD dwWaitResult;
    BOOL bContinue = TRUE;
    int check = 1;
    srand(time(0));
    GetLocalTime(&lt);


    while (bContinue)
    {
        for (int i = 0; i < 5; i++)
            for (int j = 0; j < ((rand() % MAX_TRANS_PDAY) + 1); j++)
            {

                dwWaitResult = WaitForSingleObject(
                    ghMutex,    // handle to mutex
                    INFINITE);  // infinite time-out interval

                switch (dwWaitResult)
                {
                    // The object was signaled.
                case WAIT_OBJECT_0:
                    // TODO: Perform task
                    if (displayDay && check != i)
                        printf("%s\n", bank->weekdayName(i).c_str());
                    displayDay = !displayDay;
                    check = i;

                    bContinue = FALSE;
                    bank->performTransaction();


                    // Release the mutex when task is finished

                    if (!ReleaseMutex(ghMutex))       // not interested in previous count
                    {
                        printf("ReleaseMutex Error: %d\n", GetLastError());
                    }

                    //Admin finishes Transaction
                    printf(":::::Admin %c finished transaction\n", bank->getAdminName());
                    break;

                case WAIT_ABANDONED:
                    return FALSE;
                }
            }

    }

    return TRUE;
}

DWORD WINAPI workerThread(LPVOID lpParam)
{
    DWORD dwWaitResult;
    Bank* bank = static_cast<Bank*>(lpParam);

    return usingCriticalSection(bank);
}
