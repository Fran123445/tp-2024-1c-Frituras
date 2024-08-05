# <h1 align="center">Trabajo Práctico - Sistemas Operativos UTN FRBA - 1c2024</h1>

## Enunciado y pruebas

* [Enunciado del TP](https://github.com/Fran123445/tp-2024-1c-Frituras/blob/main/Enunciado%20y%20pruebas/C%20-%20Comenta-%20v1.3.pdf)
* [Documento detallando las pruebas](https://github.com/Fran123445/tp-2024-1c-Frituras/blob/main/Enunciado%20y%20pruebas/1C2024%20-%20C-Comenta%20-%20Pruebas%20Finales.pdf)

## Entorno y herramientas utilizadas
* [VirtualBox](https://www.virtualbox.org/) - Usada para correr las máquinas virtuales
* [Visual Studio Code](https://code.visualstudio.com/) - Editor de texto elegido para codear. Usado también a la hora de debuggear.
* [Xubuntu](https://drive.google.com/drive/folders/1ASZjI4HsAKDZNgNJ-owhZA2GAJ8Rsqjr) - Imagen de Xubuntu provista por la cátedra para el desarrollo del TP.
* [Ubuntu Server](https://drive.google.com/drive/folders/1Pn1SveTGkEVfcc7dYAr1Wc10ftEe8E0J) - Imagen de Ubuntu Server provista por la cátedra para las pruebas y evaluación del trabajo.
* [Valgrind](https://valgrind.org/) - Herramienta utilizada para encontrar leaks de memoria durante el desarrollo.
* [C](https://www.gnu.org/software/gnu-c-manual/gnu-c-manual.html) - Lenguaje de programación utilizado.
* [so-commons-library] - Biblioteca proporcionada por la cátedra para el manejo de listas y otros tipos de dato abstractos.

## Equipo

| Nombre                | Módulo/s desarrollados | Cuenta de Github                                |
| --------------------- | ---------------------- | ----------------------------------------------- |
| Franco Stazzone       | Kernel                 | [Fran123445](https://github.com/Fran123445)     |
| Iván Abete            | CPU, Memoria           | [Ivanabete](https://github.com/Ivanabete)       |
| Matías Aponte         | CPU, IO                | [matuaponte](https://github.com/matuaponte)     |
| Yanel Agostini Dohmen | Memoria                | [yaanel](https://github.com/yaanel)             |
| Lucas Jorge Ocampo    | IO                     | [LucasJocampo](https://github.com/LucasJocampo) |

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

## Compilación

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante se guardará en la carpeta `bin` del módulo.

## Ejecución

Para la ejecución del proyecto, se deberán iniciar los módulos en el siguiente orden:

1. Memoria
2. CPU
3. Kernel
4. I/O

A cada uno de los primeros tres módulos se los debe ejecutar pasándole como primer argumento el correspondiente archivo de configuración. En el caso de las interfaces I/O, a estas se les debe pasar el nombre que llevará la interfaz y luego la ruta del archivo de configuración.

Por ejemplo, para ejecutar la prueba IO detallada en el documento de pruebas, los módulos se deberían levantar de la siguiente manera:

1. ./bin/memoria io.config
2. ./bin/cpu io.config
3. ./bin/kernel io.config
4. ./bin/entradasalida TECLADO teclado.config
5. ./bin/entradasalida MONITOR monitor.config
6. ./bin/entradasalida GENERICA generica.config

El orden en el cual se conectan las interfaces IO es indistinto una vez haya sido inicializado Kernel.

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library
