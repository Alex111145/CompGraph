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

CONTROLLI (mostrati anche nella legenda in alto a sinistra, testo bianco sopra la scena)
  W A S D      muovi la camera libera
  Q / E        giu' / su
  mouse        guarda in giro (camera libera)
  C            cambia camera (libera, frontale, dall'alto)
  1            accende/spegne la torcia (luce direzionale che segue la camera)
  2            accende/spegne la lampada (luce puntiforme letta dal file, nel paralume, fa l'ombra)
  F            wireframe / solido (debug: glPolygonMode GL_LINE / GL_FILL)
  P            stampa la posizione della camera attiva
  ESC          esci

All'avvio la console stampa lo scene graph con la posizione mondo di ogni
nodo e il numero di triangoli (14837 in totale).

STRUTTURA DEL MOTORE (diagrammi UML in doc/uml/)
- Eng::Base      singleton: finestra e contesto (FreeGLUT, usato solo
                 dentro il motore), input, clearWindow/render/swapBuffers.
- Eng::Object    base astratta: id univoco, nome, render().
- Eng::Node      scene graph (matrice locale, padre, figli).
  - Eng::Mesh              triangoli in display list + materiale.
  - Eng::Camera            camera prospettica, vista = inversa del mondo.
  - Eng::Light (astratta)  glLight; Eng::DirectionalLight, Eng::PointLight.
- Eng::Material, Eng::Texture   glMaterial / glBindTexture.
- Eng::List      render list: prima le luci, poi le mesh, poi l'ombra.
- Eng::Loader    file 3D (Assimp) -> scene graph: nodi, mesh, materiali,
                 texture e luci (glTF KHR_lights_punctual).
Il client non chiama mai OpenGL o FreeGLUT: usa solo le classi Eng::.

COMPILAZIONE
Tutte le dipendenze sono nella cartella dependencies/ (allo stesso livello
di engine/ e client/), niente da installare a parte il compilatore:
  glm/       GLM 1.0.3 (solo header)
  stb/       stb_image.h (solo header)
  freeglut/  FreeGLUT 3.2.2: include/GL, lib/windows/Debug|Release
             (freeglutd/freeglut .lib + .dll), lib/linux/libglut.so.3
  assimp/    Assimp 6.0.5: include/assimp, lib/windows
             (assimp-vc143-mt .lib + .dll), lib/linux/libassimp.so.6
- Windows: apri cg_Gervasini_Alessio.sln in Visual Studio 2022/2026 (x64).
  Dopo la build dell'engine le DLL di FreeGLUT e Assimp vengono copiate
  accanto a client.exe (x64/Debug o x64/Release).
- Linux: "make" dalla cartella del progetto, oppure apri
  cg_Gervasini_Alessio.workspace in Code::Blocks. Servono solo gli header
  OpenGL di sistema (libgl-dev, libglu1-mesa-dev) e le librerie X11 di un
  normale desktop Ubuntu (libxi6, libxrandr2, libxxf86vm1, usate da
  FreeGLUT). Le .so di FreeGLUT e Assimp vengono copiate accanto a
  libengine.so (engine/bin/Debug|Release); per avviare dalla cartella
  client:
  LD_LIBRARY_PATH=../engine/bin/Release ./bin/Release/client

DEMO
windows/ : lancia client.exe.
demo/    : lancia demo/run.sh (Linux x86_64, le .so di FreeGLUT e Assimp
           sono gia' nella cartella).

DOCUMENTAZIONE
"doxygen Doxyfile" dalla cartella engine produce doc/html/.
Progetto UML (diagrammi + scelta del modello 3D): doc/uml/Progetto_UML.pdf;
solo i diagrammi: doc/uml/UML_Solo_Grafici.pdf.

MODELLO 3D
"Bedroom Furniture Set - Game Ready" di General of Thailand
(https://sketchfab.com/Melonpolygons), licenza CC BY 4.0
(http://creativecommons.org/licenses/by/4.0/).
Fonte: https://sketchfab.com/3d-models/bedroom-furniture-set-game-ready-c30a07d2f8bc4f0a843f1629c5ef78c4
Modificato: aggiunta una lampada da scrivania con paralume luminoso e la
sua luce puntiforme (LampLight).
