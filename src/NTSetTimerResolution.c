#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <signal.h>
extern NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution(ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution);
extern NTSYSAPI NTSTATUS NTAPI NtQueryTimerResolution(PULONG MinimumResolution, PULONG MaximumResolution, PULONG CurrentResolution);
#define STATUS_SUCCESS 0x00000000
#define STATUS_TIMER_RESOLUTION_NOT_SET 0xC0000245

BOOLEAN isCurrentResolutionChanged = FALSE;
ULONG CurrentResolution = 0;
ULONG DesiredResolution = 5000;

void signalhandler(int signal)
{
    if(signal == SIGINT || signal == SIGTERM)
    {
        if(isCurrentResolutionChanged)
        {
            switch (NtSetTimerResolution(DesiredResolution, FALSE, &CurrentResolution)) {
                case STATUS_SUCCESS:
                    printf("The current resolution has returned to %d [100 ns units].\n", CurrentResolution);
                    break;
                case STATUS_TIMER_RESOLUTION_NOT_SET:
                    printf("The requested resolution was not set by this application.\n");   
                    break;
                default:
                    printf("Something wrong with NtSetTimerResolution call.");
            }
        }
    }
}

int main()
{
    signal(SIGINT, signalhandler);
    signal(SIGTERM, signalhandler);
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG MinimumResolution = 0;
    ULONG MaximumResolution = 0;
    if((ntStatus = NtQueryTimerResolution(&MinimumResolution, &MaximumResolution, &CurrentResolution)) != STATUS_SUCCESS)
    {
        printf("Something wrong with NtQueryTimerResolution call.");
        return ntStatus;
    }
    printf("MinimumResolution [100 ns units]: %d.\n", MinimumResolution);
    printf("MaximumResolution [100 ns units]: %d.\n", MaximumResolution);
    printf("CurrentResolution [100 ns units]: %d.\n", CurrentResolution);
    if ((ntStatus = NtSetTimerResolution(DesiredResolution, TRUE, &CurrentResolution)) != STATUS_SUCCESS)
        printf("Something wrong with NtSetTimerResolution call.");
    else
    {
        isCurrentResolutionChanged = TRUE;
        printf("New CurrentResolution [100 ns units]: %d.\n", CurrentResolution);
        printf("Press any key to restore the original resolution...\n");
        int waitFor = 0;
        waitFor = _getch();
        switch ((ntStatus = NtSetTimerResolution(DesiredResolution, FALSE, &CurrentResolution))) {
            case STATUS_SUCCESS:
                printf("The current resolution has returned to %d [100 ns units].\n", CurrentResolution);
                break;
            case STATUS_TIMER_RESOLUTION_NOT_SET:
                printf("The requested resolution was not set by this application.\n");
                break;
            default:
                printf("Something wrong with NtSetTimerResolution call.");
        }
    }
    return ntStatus;
}