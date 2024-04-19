/***************************************************
*  Programm zum Senden von IDOK an alle Fenster,   *
*  die einen angegebenen Text entalten             *
****************************************************
* Änderungen *                                     *
****************************************************
* 19.04.2024 *  aN * .01 * Start mit ChkWnd
* **************************************************/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\\tiny-regex-c\\re.h"

//** Enum *****************************************/
enum Command
{
    Nix,
    Close,
    SendMsg,
    ShwHid
};

//** Types ****************************************/

//** Prototypen ***********************************/
int StrInStr(char *sub, char *str);
int RegEx(char *sub, char *str);
int FindWindowText(char *suche);
int DoKommand(HWND hWnd, enum Command cmd);

//** Variablen ************************************/
int (*compare)(char *sub,char *str);
char wie_t[] = "text";
char wie_r[] = "regex";
char *wie = wie_t;
enum Command CmD=Nix;
char suchtit[] = "~S~e~a~r~c~h~";
HWND selbst = NULL;
int ignore_case = 0;
int cmd_id = IDOK;
int show_hide = SW_SHOW;
int dummy = 0;
char titel[500];

//*************************************************/

/*
 Suche String in String
*/
int StrInStr(char *sub, char *str)
{
    char *pos = strstr(sub,str);
    return (pos != NULL);
}

/*
 regular expression
*/
int RegEx(char *sub, char *str)
{
    int l;
    int res = re_match(str, sub, &l);
    return (res != -1);
}

/*
Kommando Ausführen
*/
int DoKommand(HWND hWnd, enum Command cmd)
{
    switch (cmd) {
    case Close:
        // Sende WM_CLOSE an das Fenster
        SendMessage(hWnd,WM_CLOSE,0,0);
        break;
    case SendMsg:
        // Sende WM_COMMAND, IDOK an das Fenster
        SendMessage(hWnd,WM_COMMAND, cmd_id, 0);
        break;
    case ShwHid:
        ShowWindow(hWnd, show_hide);
        break;
    default:
        //Nix
        break;
    }
    return 0;
}

/*
  Suchen eines Fensters. das die gegebene Zeichenfolge enthält
 */
int FindWindowText(char *txt)
{
    HWND hwnd;
    int treffer = 0;
    static char hStr[500];
    static char vg_suche[500];
    static char vg_titel[500];
    
    printf("Suche (%s): %s\n",wie,txt);
    
    strcpy(vg_suche, txt);
    if(ignore_case)
    {
        _strlwr(vg_suche);
    }

    hwnd = FindWindow(NULL, NULL);

    while(hwnd != NULL)
    {
        // - Fenster auswerten
        GetWindowText(hwnd, hStr, sizeof(hStr));
        
        strcpy(vg_titel, hStr);
        if(ignore_case)
        {
            _strlwr(vg_titel);
        }

        if (selbst != hwnd)
        {
            if ((*compare)(vg_titel, vg_suche) != 0)
            {
                printf("gefunden: %s\n", hStr);
                
                DoKommand(hwnd, CmD);
                
                treffer++;
            }
        }
        else
        {
            // printf("geigenes Fenster: %s\n", hStr);
        }
        hwnd = GetWindow(hwnd,GW_HWNDNEXT);
    }
    printf("Treffer: %d\n", treffer);
    return treffer;
}

int main(int argc, char *argv[])
{
    selbst = GetWindow(GetConsoleWindow(),GW_OWNER);
    char titel[350];
    char *srch = NULL;
    int treffer_ges = 0;
   
   compare = &StrInStr;   
    
    GetConsoleTitle(titel, sizeof(titel));

    SetConsoleTitle(suchtit);
    Sleep(100);
    
    for(int i=1;i<argc;i++)
    {
        if (argv[i][0]=='-')
        {
            switch (argv[i][1]) 
            {
            case 't':
                // Text direct
                compare = &StrInStr;
                wie = wie_t;
                break;
            case 'r':
                // regular expression
                compare = &RegEx;
                wie = wie_r;
                break;
            case 'i':
                // ignore case
                ignore_case = 1;
                break;
            case 'c':
            case 's':
                // close Window
                CmD = Close;
                break;
            case 'v':
            case 'h':
                // Show/Hide 
                CmD = ShwHid;
                if((argv[i][2]=='z')
                || (argv[i][2]=='s'))
                {
                    //Zeigen/Show
                    show_hide = SW_SHOW;
                }
                else
                if((argv[i][2]=='v')
                || (argv[i][2]=='h'))
                {
                    //Verstecken/Hide
                    show_hide = SW_HIDE;
                }
                break;
            case 'k':
                // Show/Hide 
                CmD = SendMsg;
                if(argv[i][2]==0)
                {
                    //Zeigen/Show
                    cmd_id = IDOK;
                }
                else
                {
                    cmd_id = atoi(&argv[i][2]);
                }
                break;
            default:
                //TODO
                break;
            }
        }
        else
        {
            srch = argv[i];
            treffer_ges += FindWindowText(srch);
        }
    }

    if (srch != NULL)
    {
        printf("gesamt %d Treffer gefunden\n", treffer_ges);
    }
    else
    {
        printf("Verwendung:\nChkWnd [[-r][-t][-i] <SuchText>]...\n"
            "    -r   ... regular Expression\n"
            "    -t   ... Text direkt\n"
            "    -i   ... ignoriere Groß-/Kleinschreibung\n"
            "    -s   ... Fenster schließen\n"
            "    -vx  ... Fenster verstecken x=v/z\n"
            "    -k#  ... Sende Command #/IDOK\n"
            "%%ERRORLEVEL%% ist Anzahl Treffer\n");
    }

    SetConsoleTitle(titel);

    return treffer_ges;
}
