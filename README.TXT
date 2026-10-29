COMPUTER GRAPHICS (M-I5203) - PROGETTO
Alessio Gervasini, SUPSI

DESCRIZIONE
Il progetto e' diviso in due parti:
- engine: motore grafico come libreria (engine.dll su Windows,
  libengine.so su Linux) con scene graph, camere, luci, materiali,
  texture e caricamento di modelli 3D;
- client: viewer che usa il motore per mostrare una camera da letto
  caricata da un file .glb.
Il motore disegna con OpenGL a pipeline fissa (display list, glLight,
glMaterial) e usa FreeGLUT per finestra e input. Il client usa solo le
classi del namespace Eng e non chiama mai OpenGL o FreeGLUT.

CONTROLLI (riassunti anche nella legenda in alto a sinistra)
  W A S D      muovi la camera libera
  Q / E        giu' / su
  mouse        guarda in giro (camera libera)
  C            cambia camera (libera, frontale, d'angolo)
  1            accende/spegne la torcia (luce direzionale che segue la camera)
  2            accende/spegne la lampada (luce puntiforme sulla scrivania, fa l'ombra)
  F            wireframe / solido
  P            stampa la posizione della camera attiva
  ESC          esci

All'avvio la console stampa lo scene graph: per ogni nodo id, id del
padre, posizione locale e posizione nel mondo, e il numero di triangoli
di ogni mesh.

CLASSI DEL MOTORE (namespace Eng, diagrammi UML in doc/uml/)
- Base               singleton: finestra, input, clearWindow, render,
                     renderLegend, swapBuffers.
- Object             classe base astratta: id univoco, nome, render().
- Node               nodo dello scene graph: matrice locale, padre, figli.
- Mesh               triangoli compilati in una display list + materiale.
- Camera             camera prospettica, la matrice di vista e' l'inversa
                     della sua matrice mondo.
- Light              luce astratta, con DirectionalLight e PointLight.
- Material, Texture  colori della superficie e immagine applicata.
- List               render list: prima le luci, poi le mesh, poi l'ombra.
- Loader             legge il file 3D con Assimp e crea nodi, mesh,
                     materiali, texture e luci.

SCELTE PRINCIPALI
- Matematica con GLM, vettori colonna: matrice mondo di un nodo = matrice
  mondo del padre x matrice locale.
- Ombra planare della lampada sul pavimento, con la matrice di proiezione
  dell'ombra della OpenGL SuperBible (cap. 9); lo stencil buffer la limita
  al pavimento e un piano di clipping (glClipPlane) toglie la parte degli
  oggetti che sta sopra la lampada.
- La luce della lampada e' salvata nel file .glb e letta dal Loader.
- La legenda usa un piccolo font 5x7 disegnato con GL_QUADS.

COMPILAZIONE
Le dipendenze sono nella cartella dependencies/:
  glm/       GLM 1.0.3 (solo header)
  stb/       stb_image.h (solo header)
  freeglut/  FreeGLUT 3.2.2: include/GL, lib/windows/Debug|Release
             (freeglutd/freeglut .lib + .dll), lib/linux/libglut.so.3
  assimp/    Assimp 6.0.5: include/assimp, lib/windows
             (assimp-vc143-mt .lib + .dll), lib/linux/libassimp.so.6
- Windows: aprire cg_Gervasini_Alessio.sln con Visual Studio 2022 (x64).
  Dopo la build dell'engine le DLL di FreeGLUT e Assimp vengono copiate
  accanto a client.exe (x64/Debug o x64/Release).
- Linux: "make" dalla cartella del progetto, oppure aprire
  cg_Gervasini_Alessio.workspace con Code::Blocks. Servono gli header
  OpenGL (libgl-dev, libglu1-mesa-dev) e le librerie X11 usate da FreeGLUT
  (libxi6, libxrandr2, libxxf86vm1). Le .so di FreeGLUT e Assimp vengono
  copiate accanto a libengine.so (engine/bin/Debug|Release). Per avviare
  dalla cartella client:
  LD_LIBRARY_PATH=../engine/bin/Release ./bin/Release/client

DEMO
windows/ : avviare client.exe.
demo/    : avviare demo/run.sh (Linux x86_64).

DOCUMENTAZIONE
"doxygen Doxyfile" dalla cartella engine genera doc/html/.
Diagrammi UML: doc/uml/UML_Solo_Grafici.pdf (sorgenti PlantUML .puml).

MODELLO 3D
"Bedroom Furniture Set - Game Ready" di General of Thailand
(https://sketchfab.com/Melonpolygons), licenza CC BY 4.0
(http://creativecommons.org/licenses/by/4.0/).
Fonte: https://sketchfab.com/3d-models/bedroom-furniture-set-game-ready-c30a07d2f8bc4f0a843f1629c5ef78c4
Modificato: aggiunta una lampada da scrivania con il paralume luminoso e
la sua luce puntiforme (LampLight).
