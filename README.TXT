COMPUTER GRAPHICS (M-I5203) -- README

Motore + viewer scritti usando ESCLUSIVAMENTE OpenGL 1.1/1.0 a pipeline fissa
(glBegin/glVertex, matrix stack glMatrixMode/glLoadMatrixf, luci glLight,
vertex array glVertexPointer -- nessuno shader, nessuna VAO/VBO, nessun FBO),
come richiesto dal corso ("What you will NOT learn: Graphics APIs beyond
OpenGL 1.*", cg_introduction.pdf). Al posto delle shadow map (che
richiederebbero un framebuffer object, non disponibile in OpenGL 1.1),
il requisito "ombre o riflessi" e' soddisfatto con una proiezione planare
via stencil buffer, ritagliata sulla sagoma del pavimento (individuato
per geometria -- vertici complanari piu' bassi della scena -- non per
nome della mesh).

CONTROLLI DELLA DEMO
Un pannello con i controlli appare a schermo durante l'esecuzione;
eccoli anche qui per riferimento:

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
demo/ contiene un eseguibile Linux x86_64 gia' compilato (client +
libengine.so + assets), verificato in un container Ubuntu 24.04 pulito
(render Mesa software, scena caricata correttamente: 4 mesh, 14837
triangoli). Lancia demo/run.sh da terminale.

windows/ contiene un eseguibile Windows x64 gia' compilato (client.exe +
engine.dll + assets), stessa scena verificata con lo stesso esito.
Compilato pero' con un toolchain diverso da quello ufficiale del
progetto: MinGW-w64 (cross-compiler, non MSVC) dentro Docker, perche'
su questa macchina non c'e' Visual Studio; verificato eseguendolo con
Wine invece che su Windows reale. Il .sln resta il modo di riferimento
per compilare con MSVC/vcpkg su una macchina Windows vera, seguendo la
sezione COMPILAZIONE sopra.

DOCUMENTAZIONE
Generata con Doxygen in doc/html/ (apri doc/html/index.html).
