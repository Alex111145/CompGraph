COMPUTER GRAPHICS (M-I5203) -- README

Motore grafico (engine.dll / libengine.so) + viewer (client) scritti usando
solo OpenGL 1.1/1.0 a pipeline fissa, come richiesto dal corso
("Graphics APIs beyond OpenGL 1.*" esclusi, cg_introduction.pdf).

CONVENZIONI (cg_mathForCg1.pdf, OpenGL SuperBible)
- Sistema di coordinate destrorso (right-hand rule), Y verso l'alto.
- Matematica con GLM: vettori colonna, post-moltiplicazione (M * v).
- Matrici inizializzate con l'identita' glm::mat4(1.0f), vettori con 0.
- Scene graph: matrice mondo = matrice mondo del padre x matrice locale.
- Camera: matrice di vista = inversa della matrice mondo della camera.
- Luci: direzioni normalizzate, normali normalizzate (GL_NORMALIZE).
- Mesh compilate in display list (glNewList / glBegin(GL_TRIANGLES)).
- Ombra planare con la matrice MakeShadowMatrix della SuperBible
  (cap. 9): M = (piano . luce) I - luce piano^T, proiettata dalla lampada
  (luce puntiforme) e ritagliata sul pavimento con lo stencil buffer.
- Stampa di vettori/matrici con GLM_ENABLE_EXPERIMENTAL +
  glm/gtx/string_cast.hpp (glm::to_string).

CONTROLLI (mostrati anche nel pannello in alto a destra)
  W A S D      muovi la camera libera
  Q / E        giu' / su
  mouse        guarda in giro (camera libera)
  C            cambia camera (libera, frontale, dall'alto)
  1            accende/spegne la torcia (luce direzionale che segue la camera)
  2            accende/spegne la lampada (luce puntiforme ferma nel paralume, fa l'ombra)
  P            stampa la posizione della camera attiva
  ESC          esci

All'avvio la console stampa lo scene graph con la posizione mondo di ogni
nodo e il numero di triangoli (14837 in totale).

COMPILAZIONE
- Windows: apri cg_Gervasini_Alessio.sln in Visual Studio 2022/2026 (x64).
  GLFW3 e Assimp sono gestiti da vcpkg (manifest mode, vcpkg.json).
  GLM, stb_image e stb_easy_font sono in dependencies/.
- Linux: installa libglfw3-dev e libassimp-dev, poi "make" dalla cartella
  del progetto, oppure apri cg_Gervasini_Alessio.workspace in Code::Blocks.

DEMO
windows/ : lancia client.exe.
demo/    : lancia demo/run.sh (Linux x86_64).

DOCUMENTAZIONE
"doxygen Doxyfile" dalla cartella engine produce doc/html/.
