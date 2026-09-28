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
  mouse        guarda in giro (camera libera)
  C            passa dalla camera libera a quella fissa e viceversa
  1            accende/spegne la torcia (luce direzionale che segue la camera)
  2            accende/spegne la lampada (luce puntiforme ferma sulla lampada)
  X            esci dall'applicazione

Ci sono due camere: una libera (WASDQE + mouse) e una fissa in una
posizione prestabilita della scena. Il tasto C alterna tra le due.

COMPILAZIONE
- Windows: apri cg_Gervasini_Alessio.sln in Visual Studio 2022 (v143 toolset, x64).
  Le dipendenze esterne sono GLFW3 e Assimp, gestite da vcpkg in manifest
  mode: la prima build le scarica e compila da sola (serve connessione internet
  la prima volta). GLM e' invece vendorizzata direttamente nel progetto
  in dependencies/glm/ (istruzione del corso, cg_series_1.pdf pag. 2:
  "place any required dependencies in a directory called dependencies").
- Linux: servono le dev-lib libglfw3-dev e libassimp-dev installate via
  package manager. Poi "make" dalla cartella del progetto, oppure apri
  engine/engine.cbp e client/client.cbp in Code::Blocks.

DEMO
windows/ contiene l'eseguibile Windows x64 gia' compilato (client.exe +
engine.dll + assets): lancia client.exe dalla cartella windows/.

demo/ contiene l'eseguibile Linux x86_64 gia' compilato (client +
libengine.so + assets): lancia demo/run.sh da terminale.

All'avvio la console mostra la scena caricata: 4 mesh, 14837 triangoli.

DOCUMENTAZIONE
Generabile con Doxygen: "doxygen engine/Doxyfile" produce doc/html/.
