# C-Sincronization
Proyecto 2, Sistemas Operativos ll Semestre 2017


gcc -pthread -o inicializador inicializador.c && ./inicializador "n" <br />
Donde "n" es la cantidad de memoria a reservar


gcc -pthread -o productor productor.c && ./productor "tipo" <br />
Donde "tipo" es 1 para paginacion o 2 para segmentacion

gcc -pthread -o espia espia.c && ./espia "pasadas" <br />
Donde "pasadas" es la cantidad de veces a espiar