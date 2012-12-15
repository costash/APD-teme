*** Tema 3 APD ***
*** Decembrie 2012 ***
*** Constantin Șerban-Rădoi 333CA ***

    Tema presupune implementarea paralelă a algoritmilor Julia respectiv
Mandelbrot.

    În implementarea paralelă a acestor algoritmi am împărțit imaginea în 
mai multe linii de procesat, trimițând inițial fiecărui proces informațiile 
necesare pentru a-și calcula dimensiunea chunk-ului. Acesta trimite înapoi
procesului master rezultatul sub forma unui array, ce conține un bloc din
matricea ce va compune imaginea în final. Această imagine este formată de
către procesul master în momentul în care primește toate rezultatele de la
celelalte procese.

    Tema se compune din 3 fișiere sursă:
        * Tema3.cpp     - conține implementarea de bază
        * Complex.h     - header pentru clasa de numere complexe
        * Complex.cpp   - implementarea clasei de numere complexe
    
    De asemenea am inclus și două script-uri pentru a putea rula tema pe
cluster:
        * script.sh     - rularea temei pe un singur set de date de intrare
        * run.sh        - rularea temei pentru toate datele de test.

    Scriptul script.sh primește 4 parametri:
        1) NUME_EXECUTABIL
        2) FIȘIER_INTRARE
        3) FIȘIER_IEȘIRE
        4) NUMĂR_PROCESE

    Scriptul run.sh primește 3 parametri:
        1) NUME_EXECUTABIL
        2) CALE_ABSOLUTĂ_FOLDER_TEST
        3) NUME_FIȘIER_IEȘIRE

    După rularea scriptului run.sh am observat o mică scalare de la 2 la 4,
respectiv 8 procese, pe datele de intrare mai solicitante/mari.
