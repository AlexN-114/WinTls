/***************************************************
*  Programm zum Senden von IDOK an alle Fenster,   *
*  die einen angegebenen Text entalten             *
****************************************************
* Änderungen *                                     *
****************************************************
* 19.04.2024 *  aN * .01 * Start mit ChkWnd
* 20.04.2024 *  aN * .10 * Fenster bewegen
* 23.04.2024 *  aN * .14 * Fehlermeldung bei unbekannten Kommando
* 23.04.2024 *  aN * .16 * selbststellendes Steuerzeichen
* 24.04.2024 *  aN * .17 * Hilfe in eigener Funktion
* 24.04.2024 *  aN * .18 * selbstanpassender Hilfetext
* 25.04.2024 *  aN * .19 * Action angeseigt
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
    ShwHid,
    WndMove
};

//** Types ****************************************/

//** Prototypen ***********************************/
int StrInStr(char *sub, char *str);
int RegEx(char *sub, char *str);
int FindWindowText(char *suche);
int WindowMove(HWND hwnd);
int DoKommand(HWND hWnd, enum Command cmd);
void help(void);

//** Variablen ************************************/
int (*compare)(char *sub,char *str);
char wie_t[] = "text";
char wie_r[] = "regex";
char *wie = wie_t;
enum Command CmD=Nix;
char suchtit[] = "~S~e~a~r~c~h~";
char *action = "Suche";
HWND selbst = NULL;
int ignore_case = 0;
int cmd_id = IDOK;
int show_hide = SW_SHOW;
int show_class = 0;
char strzchn = ' ';
int wndX;
int wndY;
int dummy = 0;
char titel[500];

//*************************************************/

/*
Hilfe
*/
void help()
{
    static int showwn = 0;
    static char text[] =
    "Verwendung:\nWndTls [[-r][-t][-i][-k#][-m#/#][-c][-?] <SuchText>]...\n"
    "    -r   ... regular Expression\n"
    "    -t   ... Text direkt\n"
    "    -i   ... ignoriere Groá-/Kleinschreibung\n"
    "    -s   ... Fenster schlieáen\n"
    "    -vx  ... Fenster verstecken x=v/z\n"
    "    -k#  ... Sende Command #/IDOK\n"
    "    -m#/#... Fenster bewegen posX/PosY\n"
    "    -c-  ... Fenster-Klasse anzeigen\n"
    "    -?   ... Hilfe\n"
    "%%ERRORLEVEL%% ist Anzahl Treffer\n";
    
    if('/'==strzchn)
    {
        for(int i=1;text[i]!=0;i++)
        {
            if('-'==text[i])
            {
                text[i] = (' '==text[i-1]) ? '/' : text[i];
                text[i] = ('['==text[i-1]) ? '/' : text[i];
            }
        }
    }
    if(0 == showwn)
    {
        showwn = 1;
        printf("%s",text);
    }
}

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
Fenster bewegen
*/
int WindowMove(HWND hwnd)
{
    RECT r;
    
    GetWindowRect(hwnd, &r);
    MoveWindow(hwnd,wndX,wndY,r.right-r.left,r.bottom-r.top,TRUE);
    
    return 0;
}

/*
Kommando Ausführen
*/
int DoKommand(HWND hWnd, enum Command cmd)
{
    switch (cmd) {
    case Close:
        // Sende WM_CLOSE an das Fenster
        PostMessage(hWnd,WM_CLOSE,0,0);
        break;
    case SendMsg:
        // Sende WM_COMMAND, IDOK an das Fenster
        PostMessage(hWnd,WM_COMMAND, cmd_id, 0);
        break;
    case ShwHid:
        // Show-Hide Fenster
        ShowWindow(hWnd, show_hide);
        break;
    case WndMove:
        // Fenster bewegen
        WindowMove(hWnd);
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
    static char class[100];
    static char hStr[500];
    static char vg_suche[500];
    static char vg_titel[500];
    
    printf("%s (%s): %s\n",action,wie,txt);
    
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
                GetClassName(hwnd, class, sizeof(class));
                if(show_class != 0)
                {
                    printf("gefunden:(%s) %s\n",class,hStr);
                }
                else
                {
                    printf("gefunden: %s\n", hStr);
                }
                
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
    char titel[350];
    char *srch = NULL;
    int treffer_ges = 0;
   
    selbst = GetWindow(GetConsoleWindow(),GW_OWNER);

    compare = &StrInStr;
    
    GetConsoleTitle(titel, sizeof(titel));

    SetConsoleTitle(suchtit);
    Sleep(100);
    
    for(int i=1;i<argc;i++)
    {
        if (strzchn == ' ' &&
            ((argv[i][0]=='/') || (argv[i][0]=='-')))
        {
            strzchn = argv[i][0];
        }
        
        if (argv[i][0]==strzchn)
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
                // ignore Groß-/Kleinschreibung
                if('-' == argv[i][2])
                {
                    ignore_case = 0;
                }
                else
                {
                    ignore_case = 1;
                }
                break;
            case 's':
                // schließe Fenster
                action = "Schlieáe";
                CmD = Close;
                break;
            case 'm':
                // Fenster bewegen
                action = "Verschiebe";
                CmD = WndMove;
                wndX = 100;
                wndY = 100;
                sscanf(&argv[i][2],"%d/%d",&wndX,&wndY);
                break;
            case 'v':
                // Show/Hide
                action = "Verstecke";
                CmD = ShwHid;
                if((argv[i][2]=='z')
                || (argv[i][2]=='s')
                || (argv[i][2]=='+'))
                {
                    //Zeigen/Show
                    show_hide = SW_SHOW;
                }
                else
                if((argv[i][2]=='v')
                || (argv[i][2]=='h')
                || (argv[i][2]=='-'))
                {
                    //Verstecken/Hide
                    show_hide = SW_HIDE;
                }
                break;
            case 'k':
                // Show/Hide
                action = "Sende Msg";
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
            case 'c':
                // Zeige Klasse
                show_class = argv[i][2]!='-';
                break;
            case '?':
                // Hilfe
                help();
                srch = (char*)1;
                break;
            default:
                // Fehlermeldung
                printf("unbekanntes Kommando: '-%c'\n",argv[i][1]);
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
        help();
    }

    SetConsoleTitle(titel);

    return treffer_ges;
}
