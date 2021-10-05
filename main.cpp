/*
 * Luis Antonio Montoya Morales
 * Para la materia de Fundamentos de programacion
 * Creado el 26 de Septiembre del 2021
 */

#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <limits>
#include <algorithm>

/*
 * Método de reducción Quine Mcclowsky
 * 1 Obtiene los minterminos de un circuito de n variables.
 * 2 Obtiene la agrupacion de los minterminos con igual numero de 1's
 *
*/

//Renombramiento de objetos para mejorar lectura del codigo
#define SALIR_MENU -1
#define ERROR_LECTURA -2
#define UNIDAD_BOOLEANA 0b01
using ListaMinterminos = std::vector<unsigned short>;                           //Arreglo dinámico, almacena enteros
using IndiceArreglo = std::vector<unsigned short>::iterator;                    //Indica el indice de un arreglo

struct PrimoImplicante{
    ListaMinterminos minterminos;
    unsigned short numero;
};

using GrupoUnos = std::vector<PrimoImplicante>;
using TablaPrimosImplicantes = std::vector<GrupoUnos>;                 //Indice indica grupo de unos

//Prototipos
unsigned int short obtenerNumeroVariables();                                        //Obtiene el numero de variables a evaluar
void obtenerMinterminos(ListaMinterminos&, unsigned short);                         //Obtiene la posicion de los minterminos
int leerEntradaNumerica(int);                                                       //Funcion de apoyo para la lectura de valores numericos, lanza mensajes
                                                                                    // de error en caso de argumento invalido (no interrumpe ejecucion)
void imprimirMinterminos(ListaMinterminos &);                                       //Imprime lista minterminos
void imprimirAgrupaciones(TablaPrimosImplicantes &);
void menuPrincipal(ListaMinterminos&, TablaPrimosImplicantes & ,unsigned short);          //Interfaz
TablaPrimosImplicantes* obtenerPrimosImplicantes(TablaPrimosImplicantes &);
bool entradaRangoValido(int, size_t);                                               //Determina si el valor de entrada es positivo en el rango indicado por
                                                                                    // maximo acotando mayor igual que)
void eliminarMinterminos(ListaMinterminos&, unsigned short);                        //Elimina mintermino de la lista
int iniciarReduccion(ListaMinterminos&, TablaPrimosImplicantes &, unsigned short);                        //Inicia proceso de reduccion Quine Macclowsky
unsigned short contarUnos(unsigned short, unsigned short);                          //Cuenta el numero de unos en su expresion booleana de un numero entero positivo
void limpiarTabla(TablaPrimosImplicantes&, unsigned short);

int main() {
    unsigned short numero_variables = obtenerNumeroVariables();
    ListaMinterminos tabla_minterminos;
    TablaPrimosImplicantes tabla_agrupaciones(numero_variables);
    menuPrincipal(tabla_minterminos, tabla_agrupaciones, numero_variables);
    return 0;
}

unsigned short obtenerNumeroVariables(){
    int numero_variables = 0;
    do{
        std::cout << "Inserte el numero de variables (dos o mas variables):" << std::endl;
        numero_variables = leerEntradaNumerica(0);
        if(numero_variables < 2)
            std::cout << "Intente con valor numerico positivo mayor a 2 y menor a " << std::numeric_limits<unsigned short>::max() << std::endl;
    }while(numero_variables < 2);
    return static_cast<unsigned short>(numero_variables);
}

void menuPrincipal(ListaMinterminos& lista, TablaPrimosImplicantes & tabla, const unsigned short numero_variables){
    int opcion = 0;
    do{
        std::cout <<    "Elija una opcion:" << std::endl <<
                        "0 Agregar minterminos" << std::endl <<
                        "1 Eliminar mintermino" << std::endl <<
                        "2 Iniciar reduccion Quine Mcclowsky" << std::endl <<
                        "-1 Salir" << std::endl;
        opcion = leerEntradaNumerica(ERROR_LECTURA);
        if(entradaRangoValido(opcion, 4) && opcion >= 0)
            switch (opcion) {
            case 0:
                obtenerMinterminos(lista, numero_variables);
                break;
            case 1:
                eliminarMinterminos(lista, numero_variables);
                break;
            case 2:
                opcion = iniciarReduccion(lista, tabla, numero_variables);
                break;
            }
    }while(opcion != SALIR_MENU);
}

void obtenerMinterminos(ListaMinterminos& lista, const unsigned short numero_variables){
    int mintermino = 0;
    std::cout << "Introduzca los minterminos del circuito (-1 para regresar al menu)" << std::endl;
    do{
        mintermino = leerEntradaNumerica(ERROR_LECTURA);
        if(entradaRangoValido(mintermino, static_cast<size_t>(std::pow(2, numero_variables))) && mintermino >= 0){
            if(std::find(lista.begin(), lista.end(), static_cast<unsigned short>(mintermino)) == lista.end()){
                lista.push_back(static_cast<unsigned short>(mintermino));
                std::cout << mintermino << " agregado" << std::endl;
            }
            else
                std::cout << mintermino << " ya se encuentra en la lista" << std::endl;
        }
    }while(mintermino != SALIR_MENU);
    std::sort(lista.begin(), lista.end());
    std::cout << std::endl << "Terminada la lectura de minterminos" << std::endl;
}

void eliminarMinterminos(ListaMinterminos& lista, const unsigned short numero_variables){
    int mintermino = 0;
    IndiceArreglo posicion_encontrado;
    imprimirMinterminos(lista);
    std::cout << "Indique mintermino a eliminar (-1 para salir)" << std::endl;
    do{
        imprimirMinterminos(lista);
        mintermino = leerEntradaNumerica(ERROR_LECTURA);
        if(entradaRangoValido(mintermino, static_cast<size_t>(std::pow(2, numero_variables))) && mintermino >= 0) {
            posicion_encontrado = std::find(lista.begin(), lista.end(), static_cast<unsigned short>(mintermino));
            if(posicion_encontrado != lista.end()){
                std::cout << "Mintermino " << mintermino << " eliminado" << std::endl;
                lista.erase(posicion_encontrado);
            }
            else
                std::cout << "Mintermino no encontrado" << std::endl << std::endl;
        }
    }while(mintermino != SALIR_MENU);
}

int iniciarReduccion(ListaMinterminos& lista, TablaPrimosImplicantes & tabla, unsigned short numero_variables){
    //Recorre la lista de minterminos, cuenta el numero de unos del mintermino y agrega a la tabla dicho mintermino
    //La tabla tiene tamaño de n variables, y el indice i indica la agrupacion con i + 1 unos
    TablaPrimosImplicantes *tmp;
    GrupoUnos grupo_tmp;
    PrimoImplicante primo_tmp;
    ListaMinterminos lista_tmp;
    unsigned short indice = 0;

    if(lista.empty())
        std::cout << "Lista minterminos vacia, nada que procesar." << std::endl;
    else{
        imprimirMinterminos(lista);
        std::cout << std::endl;
        for(unsigned short i : lista){
            indice = contarUnos(i, numero_variables) - 1;
            primo_tmp.minterminos.push_back(i);
            primo_tmp.numero = i;
            if(!tabla[indice].empty())
                tabla[indice].emplace_back(primo_tmp);
            else{
                grupo_tmp.push_back(primo_tmp);
                tabla.insert(tabla.begin() + indice, grupo_tmp);
            }
            primo_tmp.minterminos.clear();
            grupo_tmp.clear();
        }
        tabla.shrink_to_fit();
        limpiarTabla(tabla, numero_variables);
        imprimirAgrupaciones(tabla);
        //Obtencion de los primos implicantes
        //Desde la tabla principal de agrupaciones de unos
        tmp = obtenerPrimosImplicantes(tabla);
        imprimirAgrupaciones(*tmp);
    }
    return -1;
}

void limpiarTabla(TablaPrimosImplicantes& tabla, unsigned short numero_variables){
    while(tabla.size() !=numero_variables)
        tabla.pop_back();
}

TablaPrimosImplicantes* obtenerPrimosImplicantes(TablaPrimosImplicantes & tabla, unsigned short numero_variables){
    auto *tmp = new TablaPrimosImplicantes(numero_variables - 1);
    GrupoUnos grupo_tmp;
    PrimoImplicante primo_tmp;
    ListaMinterminos lista_tmp;
    for(int i = 0; i < tabla.size() - 1; ++i){
        for(int j = 0; j < tabla[i].size(); ++j){
            for(int k = 0; k < tabla[i][j].minterminos.size(); ++k){
                if(contarUnos(tabla[i][j].numero^tabla[i+1][k].numero, numero_variables) == 1){
                    primo_tmp.minterminos.insert(primo_tmp.minterminos.end(), tabla[i][j].minterminos.begin(), tabla[i][j].minterminos.end());
                    primo_tmp.minterminos.insert(primo_tmp.minterminos.end(), tabla[i+1][k].minterminos.begin(), tabla[i+1][k].minterminos.end());
                    primo_tmp.numero = contarUnos(tabla[i][j].numero^tabla[i+1][k].numero, numero_variables);
                    grupo_tmp.push_back(primo_tmp);
                    primo_tmp.minterminos.clear();
                }
            }
            tmp->insert(tmp->begin() + i, grupo_tmp);
            grupo_tmp.clear();
        }
    }
    return tmp;
}

void imprimirMinterminos(ListaMinterminos& lista){
    std::cout << "Lista minterminos: " << std::endl;
    for(auto& i : lista)
        std::cout << i << " ";
    std::cout << std::endl;
}

void imprimirAgrupaciones(TablaPrimosImplicantes & tabla){
    for(int i = 0; i < tabla.size(); ++i){
        std::cout << "Agrupaciones con " << (i + 1) << std::endl;
        for(auto & j : tabla[i]) {
            imprimirMinterminos(j.minterminos);
            std::cout << "\tnumero " << j.numero << std::endl;
        }
    }
}

bool entradaRangoValido(int numero, size_t maximo){
    if(numero <= ERROR_LECTURA || numero >= maximo){
        std::cout << "Intente con valor numerico positivo mayor a 0 y menor a " << maximo << std::endl;
        return false;
    }
    return true;
}

int leerEntradaNumerica(const int retorno_default){
    int valor_leido = 0;
    try {
        std::cin >> valor_leido;
    }catch (std::invalid_argument &exc){
        std::cerr << ("Argumento invalido: ") << exc.what() <<std::endl;
        return retorno_default;
    }
    return valor_leido;
}

unsigned short contarUnos(unsigned short numero, const unsigned short numero_variables){
    unsigned short contador = 0;
    for(int i = 0; i < numero_variables; ++i){
        contador += numero&UNIDAD_BOOLEANA;
        numero >>= 1;
    }
    return contador;
}