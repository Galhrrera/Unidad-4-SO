# os-2022-10-u3-ev1-Galhrrera

## Explicación: Como resolviste cada requisito de la aplicación:

En resumidas palabras, para lograr e funcionamiento correcto de la aplicación (los procesos de **cliente** y **servidor**), se utilizó una **Cola de mensajes**, para la respectiva comunicación entre los procesos a través del SO y se utilizaron **hilos**, para evitar que alguno de los procesos se quedara *congelado*, ya sea mientras leía los comandos ingresados por consola o enviaba los mensajes a la cola de mensajes.

Tanto en el cliente como en el servidor se crearon **2** métodos:
1. Uno para la escritura (enviar mensajes a la cola de mensajes)
2. Uno para la lectura (leer la terminal asociada al proceso correspondiente).

Como se mencionó, se utilizaron hilos, los cuales, ejecutarían en cada proceso cada uno de los métodos.

Ahora, en cunato a la **comunicación**
El servidor leerá **todos** los tipos de mensaje de la cola de mensajes, mientras que el cliente solo leerá los mensaes cuyo tipo sea el **PID** correspondiente al proceso (ya que recordemos, pueden haber múltiples procesos de cliente,, cada uno, con un PID distinto).

El cliente, una vez es iniciado, envía un mensaje al servidor que incluye su **PID** (del cliente), el cual, será almacenado en el servidor.

Los clientes envían diversos tipos de mensaje a la cola de mensaje, desde tipo **2** hasta tipo **5**, los cuales, son asignados dependiendo del **comando** ingresado en la terminal.

El servidor lee todos los tipos de mensajes y, a la hora de escrbir un mensaje en la cola, itera sobre los **PID** de los clientes que tiene almacenados con la lógica específica de cada uno de los comandos, según corresponda.

## Cómo se compila y ejecuta la aplicación

#### Compilación y ejecución del servidor

En el folder del proecto, para su compilación:
~~~
sudo gcc event.c eventList.c server.c -o server -lpthread  
~~~

En el folder del proyecto, para su ejecución:
~~~
sudo ./server  
~~~

#### Compilación y ejecución del cliente

En el folder del proecto, para su compilación:
~~~
sudo gcc event.c eventList.c client.c -o customer -lpthread
~~~

En el folder del proyecto, para su ejecución:
~~~
sudo ./customer
~~~

## Indica cuantos y el números de los vectores de prueba que estás cumpliendo

1. El servidor atiende una cantidad arbitraria de clientes. **PARCIAL**
2. El servidor muestra cada petición que realiza un cliente mostrando en la terminal la identificación del cliente y el contenido de la petición. **CUMPLE**
3. El servidor soporta concurrencia (escuchar peticiones y comandos locales al mismo tiempo). **CUMPLE**
4. El servidor crea correctamente eventos con su lista de interesados. **CUMPLE**
5. El servidor destruye correctamente eventos y sus recursos asociados. **CUMPLE**
6. El servidor recibe la orden de publicar un evento y lo publica correctamente a todos los interesados. **CUMPLE**
7. El servidor lista todos los clientes suscritos a un evento. **CUMPLE**
8. El cliente soporta concurrencia (escuchar notificaciones y comandos locales al mismo tiempo). **CUMPLE**
9. El cliente muestra en la terminal la notificación de los eventos. **CUMPLE**
10. El cliente le pregunta al servidor y muestra en la terminal todos los eventos disponibles en el servidor. **CUMPLE**
11. El cliente lista todos los eventos a los cuales está suscrito. **CUMPLE**
12. El cliente se suscribe a un evento. **CUMPLE**
13. El cliente se desuscribe a un evento. **CUMPLE**
14. En el server el comando exit funciona correctamente. Los clientes terminan al igual que el servidor. TODOS los recursos creados en el sistema operativo son liberados. **CUMPLE**
15. El cliente gestiona correctamente el comando exit en el servidor. **CUMPLE**

## URL de YouTube de un video en el que se muestren los vectores de prueba funcionando:

[Vectores funcionando](https://www.youtube.com/watch?v=VQMYViRlo_Q)

## Adicional

En el repositorio se encuentra también un folder de imágenes y videos. 
Estas imágenes y videos son la evidencia que se utilizó para la edición del video publicado de YouTube (Enunciados de cada vector de prueba con un clip respectivo para cada vector y el video final).
