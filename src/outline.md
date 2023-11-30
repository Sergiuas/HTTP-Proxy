# Porturi

Porturile sunt conceptul ce ne ajuta sa facem multiplexare la nivel de aplicatie. In contextul retelelor de comunicatie, un port este un numar asociat unei aplicatii (nu unui host). Daca o aplicatie doreste sa comunice cu alte aplicatii (aflate pe masini diferite sau aplicatii ce ruleaza local), aceasta expune un port, o locatie logica prin care accepta conexiuni si prin care se realizeaza schimbul de date. 
Pentru a identifica o aplicatie cu care vrem sa comunicam este nevoie de adresa IP a masinii pe care ruleaza si portul deschis de aplicatie. De exemplu, prin localhost:80 solicitam accesul la aplicatia de pe masina locala ce ofera servicii web. 


# Sockets

Un socket este un canal generalizat de comunicare intre procese, reprezentat in Unix print-un descriptor de fisiere*. El ofera posibilitatea de comunicare intre procese aflate pe masini diferite intr-o retea.
Un file descriptor este un handle prin care un proces comunica cu resursele sistemului. 

# TCP



TCP (Transport Control Protocol) este un protocol ce furnizează transmisie garantată (cât timp există conexiune), în ordine și o singură dată, a octeţilor de la transmiţător la receptor. Acest protocol asigură stabilirea unei conexiuni între cele două calculatoare pe parcursul comunicaţiei, și este descris în RFC 793. Protocolul TCP are următoarele proprietăţi:

    -stabilirea unei conexiuni între client și server; serverul va aștepta apeluri de conexiune din partea clienților
    -garantarea ordinii primirii mesajelor şi prevenirea pierderii pachetelor
    -controlul congestiei (fereastră glisantă)
    -overhead mai mare în comparaţie cu UDP (are un header de 20 Bytes, spre deosebire de UDP, care are doar 8 Bytes).

 Explicaţii header:

    -portul sursă este ales random de către maşina sursă a -pachetului, dintre porturile libere existente pe acea maşină
    -portul destinaţie este portul pe care maşina destinaţie poate recepţiona pachete
    checksum este valoarea sumei de control pentru un pachet TCP

sockets api for tcp
functions used: socket, bind, listen, connect, accept, send, receive, close


