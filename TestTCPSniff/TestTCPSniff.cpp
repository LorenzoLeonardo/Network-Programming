#define  UNICODE
#define  _WIN32_WINNT  0x0500

#include <stdio.h>
#include <windows.h>
#include <iptypes.h>
#include "winternl.h"
#include "tdiinfo.h"
//#include "tdistat.h"
#include "tcpioctl.h"


/*  Function:              GetTCPHandle
    Description:
      Opens a handle to the TCP driver
    Parameters:
      pTCPDriverHandle --  Pointer to a handle variable.
    Return Value (DWORD):  Returns TRUE if successful, and places
                           a valid handle to the TCP driver in the
                           handle pointed to by pTCPDriverHandle, or
                           returns FALSE otherwise, and sets the
                           handle to INVALID_HANDLE_VALUE.
*/
DWORD GetTCPHandle(PHANDLE pTCPDriverHandle)
{
#define FILE_OPEN_IF                    0x00000003
#define FILE_SYNCHRONOUS_IO_NONALERT    0x00000020
#define OBJ_CASE_INSENSITIVE            0x00000040L

    typedef NTSTATUS(NTAPI* P_NT_CREATE_FILE)(
        OUT PHANDLE              FileHandle,
        IN  ACCESS_MASK          DesiredAccess,
        IN  POBJECT_ATTRIBUTES   ObjectAttributes,
        OUT PIO_STATUS_BLOCK     IoStatusBlock,
        IN  PLARGE_INTEGER       AllocationSize OPTIONAL,
        IN  ULONG                FileAttributes,
        IN  ULONG                ShareAccess,
        IN  ULONG                CreateDisposition,
        IN  ULONG                CreateOptions,
        IN  PVOID                EaBuffer OPTIONAL,
        IN  ULONG                EaLength);

    HINSTANCE hNtDLL;
    P_NT_CREATE_FILE pNtCreateFile;
    NTSTATUS rVal;
    WCHAR TCPDriverName[] = DD_TCP_DEVICE_NAME;

    OBJECT_ATTRIBUTES  objectAttributes;
    IO_STATUS_BLOCK    ioStatusBlock;
    UNICODE_STRING     UnicodeStr;

    *pTCPDriverHandle = INVALID_HANDLE_VALUE;

    if ((hNtDLL = LoadLibrary(L"ntdll")) == NULL)
        return(FALSE);

    pNtCreateFile = (P_NT_CREATE_FILE)GetProcAddress(hNtDLL,
        "NtCreateFile");
    if (pNtCreateFile == NULL)
        return(FALSE);

    UnicodeStr.Buffer = TCPDriverName;
    UnicodeStr.Length = (USHORT)(wcslen(TCPDriverName) * sizeof(WCHAR));
    UnicodeStr.MaximumLength = UnicodeStr.Length + sizeof(UNICODE_NULL);

    objectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    objectAttributes.ObjectName = &UnicodeStr;
    objectAttributes.Attributes = OBJ_CASE_INSENSITIVE;
    objectAttributes.RootDirectory = NULL;
    objectAttributes.SecurityDescriptor = NULL;
    objectAttributes.SecurityQualityOfService = NULL;

    rVal = pNtCreateFile(pTCPDriverHandle,
        SYNCHRONIZE | GENERIC_EXECUTE,
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0);

    if (rVal < 0)
    {
        printf("\nFailed to create TCP Driver handle; NT status code = %d.", rVal);
        *pTCPDriverHandle = INVALID_HANDLE_VALUE;
        return(FALSE);
    }
    return(TRUE);
}


/*  Function:              GetEntityList
    Description:
      Allocates a buffer for and retrieves an array of TDIEntityID
    structures that identifies the entities supported by
    the TCP/IP device driver.
    Parameters:
      TCPDriverHandle  --  An open handle to the TCP Driver; if
            no such handle is available,
            may be INVALID_HANDLE_VALUE.
      lplpEntities     --  Pointer to a buffer that contains
            the array of TDIEntityID structures.
            Must be freed by the calling process
            using LocalFree( ).
    Return Value:
      DWORD  --  the number of entity structures in the returned array
*/
DWORD GetEntityArray(IN HANDLE TCPDriverHandle,
    OUT TDIEntityID** lplpEntities)
{
    TCP_REQUEST_QUERY_INFORMATION_EX  req;
    DWORD arrayLen = sizeof(TDIEntityID) * MAX_TDI_ENTITIES;
    DWORD bufferLen = arrayLen;
    TDIEntityID* pEntity = NULL;
    NTSTATUS status = 0;//TDI_SUCCESS;
    DWORD temporaryHandle = 0;
    int i;


    // First, if the handle passed in is not valid, try to obtain one.
    if (TCPDriverHandle == INVALID_HANDLE_VALUE)
    {
        if (GetTCPHandle(&TCPDriverHandle) == FALSE)
        {
            *lplpEntities = NULL;
            return(0);
        }
        temporaryHandle = TRUE;
    }

    // Next, set up the input structure for the IOCTL operation.
    req.ID.toi_entity.tei_entity = GENERIC_ENTITY;
    req.ID.toi_entity.tei_instance = 0;
    req.ID.toi_class = INFO_CLASS_GENERIC;
    req.ID.toi_type = INFO_TYPE_PROVIDER;
    req.ID.toi_id = ENTITY_LIST_ID;


    // The loop below is defensively engineered:
    // (1)  In the first place, it is unlikely that more 
    //     than MAX_TDI_ENTITIES of TCP/IP entities exist, 
    //     so the loop should execute only once.
    // (2)  Execution is limited to 4 iterations to rule out 
    //     infinite looping in case of parameter corruption. Only 2
    //     iterations should ever be necessary unless entities are 
    //     being added while the loop is running.
    for (i = 0; i < 4; ++i)
    {
        if (pEntity != NULL)
        {
            LocalFree(pEntity);
            pEntity = NULL;
            bufferLen = arrayLen;
        }

        if (arrayLen == 0)
            break;

        pEntity = (TDIEntityID*)LocalAlloc(LMEM_FIXED, bufferLen);
        if (pEntity == NULL)
        {
            arrayLen = 0;
            break;
        }

        if (!DeviceIoControl(TCPDriverHandle, // Handle to TCP driver
            IOCTL_TCP_QUERY_INFORMATION_EX, // Cmd code
            &req,            // Pointer to input buffer
            sizeof(req),     // Size of ipt buffer
            pEntity,         // Ptr to output buffer
            bufferLen,       // Size of output buffer
            &arrayLen,       // Actual size of array
            NULL))
            status = GetLastError();

        // Even if the output buffer is too small, the TCP driver 
        // returns a status of TDI_SUCCESS; it is the value returned in
        // arrayLen that indicates whether the entire array was 
        // successfully copied to the output buffer.
        if (status == 0)//TDI_SUCCESS)
        {
            if (arrayLen && (arrayLen <= bufferLen))
                break;
        }
        else
            arrayLen = 0;
    }
    if (temporaryHandle)
        CloseHandle(TCPDriverHandle);

    *lplpEntities = pEntity;
    return((DWORD)(arrayLen / sizeof(TDIEntityID)));
}

int main()
{
    DWORD i;
    DWORD entityCount;
    TDIEntityID
        * entityArray,
        * entityPtr;
    DWORD lstError = 0;

    if (!(entityCount = GetEntityArray(INVALID_HANDLE_VALUE,
        &entityArray)))
    {
        lstError = GetLastError();
        return(1);
    }

    entityPtr = entityArray;
    printf("\n\nList of %d Transport Driver Interface Entities on this machine:\n", entityCount);

    for (i = 0; i < entityCount; ++i)
    {
        printf("\n  Entity #%d:\n    Category (tei_entity) is ", i);
        switch (entityPtr->tei_entity)
        {
        case GENERIC_ENTITY:
            printf("Generic.");
            break;
        case CL_NL_ENTITY:
            printf("Connectionless Network-Layer (CL_NL)");
            break;
        case CO_NL_ENTITY:
            printf("Connected Network-Layer (CO_NL)");
            break;
        case CL_TL_ENTITY:
            printf("Connectionless Transport-Layer (CL_TL)");
            break;
        case CO_TL_ENTITY:
            printf("Connected Transport-Layer (CO_TL)");
            break;
        case AT_ENTITY:
            printf("Address Translation (AT)");
            break;
        case IF_ENTITY:
            printf("Interface (IF)");
            break;
        case ER_ENTITY:
            printf("Echo Request/Response (ER)");
            break;
        default:
            printf("[Unidentified Entity Type] = 0x%x",
                entityPtr->tei_entity);
        }
        printf("\n Instance (tei_instance) = %d\n",
            entityPtr->tei_instance);

        ++entityPtr;
    }

    //  Free the entity-array buffer before quitting.
    LocalFree(entityArray);

    return(0);
}
