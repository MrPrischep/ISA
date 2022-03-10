Tento archiv obsahuje následující soubory:
  -isa.lua
  -isa.pcap
  -myClient.cpp
  -Makefile
  -README
  -manual.pdf

isa.pcap
Výsledek komunikace mezi klientem a serverem, který je zachycen pomocí Wiresharku.

isa.lua
Dissector v jazyce Lua pro podporu nástroju Wireshark pro protokol isaProto. 

manual.pdf
Instrukce obsahující popis, pracovní princip a hlavní způsoby interakce.

myClient.cpp
Hlavní soubor implementace klienta, který bude komunikovat se serverem. Chcete-li spustit program, musíte spustit Makefile pomoci příkazu "make".
Spuštení možně ve formatu:  "./myClient [ <option> ... ] <command> [<args>]..."
  <option> is one of:
     -a <addr>, —address <addr> - Název serveru nebo adresa, ke které se chcete připojit
     -p <port>, —port <port> - Port serveru pro připojení k
     -help, -h - Zobrazit tuto nápovědu
  Supported commands:
    register <username> <password> - registruje uživatele s daným jmenem a heslem
    login <username> <password> - umožňuje přihlášení s daným přihlašovacím jménem a heslem
    list - zobrazí seznam odeslaných zpráv
    send <recipient> <subject> <body> - odeslat zprávu
    fetch <id> - najít zprávu podle čísla
    logout - odhlásit se

Makefile
Makefile pro myClient.cpp


Poznamka
V poskytnutém kliente byla k dispozici možnost využití <option> "--".
	"Nepovažujte žádný zbývající argument za přepínač (na této úrovni)
	 Více jednopísmenných přepínačů lze kombinovat po jednom -. Například-h - je stejný jako-h —."
V implementace myClient tato možnost není podporována.