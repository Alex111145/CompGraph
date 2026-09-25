COMPUTER GRAPHICS (M-I5203) -- README

Motore + viewer scritti usando ESCLUSIVAMENTE OpenGL 1.1/1.0 a pipeline fissa
(glBegin/glVertex, matrix stack glMatrixMode/glLoadMatrixf, luci glLight,
vertex array glVertexPointer -- nessuno shader, nessuna VAO/VBO, nessun FBO),
come richiesto dal corso ("What you will NOT learn: Graphics APIs beyond
OpenGL 1.*", cg_introduction.pdf). Al posto delle shadow map (che
richiederebbero un framebuffer object, non disponibile in OpenGL 1.1),
il requisito "ombre o riflessi" e' soddisfatto con un riflesso planare
classico via stencil buffer, ritagliato sulla sagoma della mesh "Ground".

CONTROLLI DELLA DEMO
Nessuna informazione sui tasti appare a schermo (solo FPS nel titolo della
finestra), quindi eccola qui:

  W A S D      muovi la camera libera (avanti/indietro/sinistra/destra)
  Q / E        muovi la camera libera in basso/alto
  frecce su/giu muovi la camera libera in alto/basso (come Q/E)
  mouse        guarda in giro (camera libera)
  C            passa dalla camera libera a quella fissa e viceversa
  1            accende/spegne la luce statica (direzionale)
  2            accende/spegne la luce dinamica (puntiforme, sulla lampada)
  X            esci dall'applicazione

Ci sono due camere: una libera (WASDQE + mouse) e una fissa in una
posizione prestabilita della scena. Il tasto C alterna tra le due.

COMPILAZIONE
- Windows: apri cg_Gervasini_Alessio.sln in Visual Studio 2022 (v143 toolset, x64).
  L'unica dipendenza esterna e' GLFW3, gestita da vcpkg in manifest mode:
  la prima build la scarica e compila da sola (serve connessione internet
  la prima volta). GLM e' invece vendorizzata direttamente nel progetto
  in dependencies/glm/ (istruzione del corso, cg_series_1.pdf pag. 2:
  "place any required dependencies in a directory called dependencies").
- Linux: serve la sola dev-lib libglfw3-dev installata via package
  manager. Poi "make" dalla cartella del progetto, oppure apri
  engine/engine.cbp e client/client.cbp in Code::Blocks.

DEMO
demo/run.sh lancia ./client con LD_LIBRARY_PATH=. (serve libengine.so
nella stessa cartella). La cartella non contiene binari: vanno copiati
li' dopo aver compilato per Linux, seguendo la sezione COMPILAZIONE
sopra.

DOCUMENTAZIONE
Generata con Doxygen in doc/html/ (apri doc/html/index.html).
