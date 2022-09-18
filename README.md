# SOL-Project
Project for the course of Operating Systems
file storage server in cui la memorizzazione dei file avviene in memoria
principale. La capacità dello storage è fissata all’avvio e non varia dinamicamente durante l’esecuzione del server.
Per poter leggere, scrivere o eliminare file all’interno del file storage, il client deve connettersi al server ed utilizzare
una API che dovrà essere sviluppata dallo studente (come descritto nel seguito). Il server esegue tutte le operazioni
richieste dai client operando sempre e solo in memoria principale e mai sul disco.
La capacità del file storage, unitamente ad altri parametri di configurazione, è definita al momento dell’avvio del
server tramite un file di configurazione testuale.
Il file storage server è implementato come un singolo processo multi-threaded in grado di accettare connessioni
da multipli client. Il processo server dovrà essere in grado di gestire adeguatamente alcune decine di connessioni
contemporanee da parte di più client.
Ogni client mantiene una sola connessione verso il server sulla quale invia una o più richieste relative ai file
memorizzati nel server, ed ottiene le risposte in accordo al protocollo di comunicazione “richiesta-risposta”. Un file è
identificato univocamente dal suo path assoluto.