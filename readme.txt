324CC Cioban George-Adrian
Protocale de Comunicatie 
Tema 2

Timp de implemenentare: 2 Zile (cumulat) de lucru 

Resurse folosite:
    // Schelet de cod (Lab 6, 7, 8)
    - Laboratoarele de Protocoale de Comunicație
    // Utilizare poll()
    - https://man7.org/linux/man-pages/man2/poll.2.html
    - https://www.ibm.com/docs/en/i/7.3?topic=designs-using-poll-instead-select
    // Dezactivare alg. Neagle
    - https://stackoverflow.com/questions/31997648/where-do-i-set-tcp-nodelay-in-this-c-tcp-client
Explicatii pentru rezolvarea temei:

    Descrierea claselor folosite

    (0) Server - Clasa folosita pentru a implementa partea de server

        Destructorul este apelat automat la iesirea din scope
        Acesta se ocupa cu inchiderea tuturor socket-urilor

        Diversele campuri si metode din aceasta clasa sunt
        descrise in comentariile din cod

        Metode mai importante:

        - handlePolls
        
        Metoda executata doar atunci cand s-a primit un raspuns
        pe cel putin unul dintre socket-urile conectate la server

        Proceseaza input de la toti clientii (TCP si UDP) dar si
        comenzi de la tastatura

        - createClient

        Metoda folosita pentru a adauga un client nou in lista de
        clienti a serverului, verifica daca id-ul nu este inregistrat
        deja, trimite toate mesajele restante unui client care s-a
        reconectat

        - getClientCommand

        Proceseaza diversele comenzi venite de la clienti (sub cu/fara sf,
        unsubscribe)

        - broadcastMsg

        Metoda ce primeste un mesaj si il trimite la toti clientii
        interesati, iar pentru clientii offline dar cu store enabled
        adauga un pointer la structura de tip storedMsg [1] in lista
        cu mesaje in asteptare a fiecarui client

    [1] Structura storedMsg contine:
        - un int ce reprezinta numarul de clienti ce mai trebuie sa primeasca
          mesajul
        - mesajul in sine

        Pentru fiecare mesaj este creata o singura structura de acest tip
        iar pointerul este adaugat la lista de mesaje in asteptare a fiecarui
        client offline dar cu SF enabled pentru topicul mesajului.
        De fiecare data cand un client primeste mesajul restant, int-ul reach
        este decrmentat iar cand ajunge la 0 memoria ocupata de structura este
        eliberata. Astfel am decis sa implementez functionalitatea de store
        and forward fara sa ocup mai multa memorie decat este necesar

    (1) Client - Clasa folosita pentru a descrie un client TCP, folosita in 
        implementarea serverului

        Detalii despre campuri au fost lasate ca si comentarii in
        ClientClass.cpp
    
    (2) Topic - Clasa folosita pentru a grupa toti subscriberii unui topic
    *   Am vrut ca aceasta clasa sa cuprinda initial mai multe functionalitati
        insa acestea au fost mutate in alte clase/ implementate in alte moduri
        de aceea a ramas un singur camp (lista de clienti ce au dat subscribe)

    Descrierea protocolului de comunicatie Server-Client TCP

    +---------------------------------+---------------------------------+
    | Server                          | Client                          |
    +=================================+=================================+
    | Deschide un socket de ascultare |                                 |
    | pentru conexiuni noi            |                                 |
    +---------------------------------+---------------------------------+
    |                                 | Trimite o cerere de conectare   |
    |                                 | catre server urmata imediat de  |
    |                                 | un string ce contine ID-ul      |
    +---------------------------------+---------------------------------+
    | Serverul confirma clientului    | Clientul asteapta confirmare de |
    | ca s-a conectat cu succes       | la server, in cazul in care     |
    | trimitand un message cu type    | primeste DENIED programul se    |
    | setat pe 6 (ACCEPT) altfel      | termina                         |
    | trimite 7 (DENIED)              |                                 |
    +---------------------------------+---------------------------------+
    | Serverul preia comanda de la    | Clientul poate trimite comenzi  |
    | client si face operatiunile     | de tip subscribe/unsubscribe    |
    | necesare (ex: adaugare in       |                                 |
    | lista de subscriberi la un      |                                 |
    | topic)                          |                                 |
    +---------------------------------+---------------------------------+
    | Serverul primeste un mesaj de   | Clientul afiseaza la STDOUT     |
    | la un client UDP si itereaza    | mesajele primite de la server   |
    | prin toti clientii ce au dat    |                                 |
    | subscribe la topicul mesajului  |                                 |
    +---------------------------------+---------------------------------+