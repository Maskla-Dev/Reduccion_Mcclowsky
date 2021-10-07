/*
 * Luis Antonio Montoya Morales
 * An assigment for Fundamentos de Programacion
 * Last Update October, 5th, 2021
 */

#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>

/*
 * Quine Mcclowsky Algorithm
 * 1 Get n minterms.
 * 2 Set group list with the same bit number
 * 3 Get expressions with one bit of difference, set repeated bit as "don't care"
 * 4 Repeat step 3 until regrouping will impossible, step 4 is named "getting prime implicants"
 * 5 Cleans prime implicants table
 * 6 Builds essential prime implicants
 *      6.a If is impossible build it, choose final prime implicants table as result table.
 * 7 Prints result on screen
 *
*/

#define ERROR_LECTURA -2
#define SALIR_MENU -1

using ushort = unsigned short;
using NumberList = std::vector<ushort>;         //Index NumberList

struct Prime_Implicant{
    NumberList minterms;
    std::string simplified_number;
    bool was_combined = false;
};

using BitGroup = std::vector<Prime_Implicant>;  //Index Prime Implicant
using MasterTable = std::vector <BitGroup>;     //Index BitGroups
using ImplicantTable = std::vector<std::vector<bool>>; //Bi-dimensional array
using IndiceArreglo = NumberList::iterator;

MasterTable* groupByBits(MasterTable&);
ushort countBits(std::string&);
bool isSameTable(MasterTable&, MasterTable*);
std::string numberToExtendedBinaryString(ushort, ushort);
void setBinaryExpressionZeros(std::string&, ushort);
int getUniqueChangePosition(std::string&, std::string&);
std::string setDontCareBit(std::string, unsigned short);
void getImplicantFromMaster(MasterTable&, BitGroup&);
BitGroup makeImplicantTable(MasterTable&, NumberList&,ImplicantTable&);
int findFirstMintermFromList(NumberList&, ushort);
void printImplicantTable(ImplicantTable&);
std::string getLettersFromImplicants(std::string&, ushort);
bool existInTable(BitGroup&, std::string&);
//--------------------------
unsigned short obtenerNumeroVariables();
int leerEntradaNumerica(int);
bool entradaRangoValido(int, size_t);
void imprimirMinterminos(NumberList &);
void obtenerMinterminos(NumberList &, unsigned short);
void eliminarMinterminos(NumberList &, unsigned short);
int initReduction(NumberList&, ushort, MasterTable&);
void menuPrincipal(NumberList&, ushort, MasterTable&);

int main(){
    ushort input_size = obtenerNumeroVariables();
    NumberList minterms;
    MasterTable table(input_size + 1);
    menuPrincipal(minterms, input_size, table);
    return 0;
}

int initReduction(NumberList& minterms, ushort input_size, MasterTable& table){
    Prime_Implicant tmp;
    MasterTable *tmp_table;
    ImplicantTable implicant_table;
    BitGroup essential_implicants;
    //First Iteration
    for(auto& i : minterms){
        tmp.minterms.push_back(i);
        tmp.simplified_number = numberToExtendedBinaryString(i, input_size);
        table[countBits(tmp.simplified_number)].push_back(tmp);
        tmp.minterms.clear();
    }
    //Second Iteration
    while(table.size() >= 2){
        tmp_table = groupByBits(table);
        if(!isSameTable(table, tmp_table)){
            table = *tmp_table;
            delete tmp_table;
            tmp_table = nullptr;
        }
        else
            break;
    }
    essential_implicants = makeImplicantTable(table, minterms, implicant_table);

    if(!essential_implicants.empty()){
        for(int i = 0; i < essential_implicants.size(); ++i){
            std::cout << getLettersFromImplicants(essential_implicants[i].simplified_number, input_size);
            if(i != essential_implicants.size() - 1)
                std::cout << " + ";
        }
    }
    else{
        BitGroup implicants;
        NumberList points(implicants.size(), 0);
        int position = 0;
        getImplicantFromMaster(table, implicants);
        //Deletes all minterms reached by prime implicants
        for(int i = 0; i < implicants.size(); ++i){
            for(int j = 0; j < implicants[i].minterms.size(); ++j){
                for(int k = i + 1; k < implicants.size() - 1; ++k)
                    for(int l = 0; l < implicants[k].minterms.size(); ++l)
                        if(implicants[i].minterms[j] == implicants[k].minterms[l])
                            implicants.erase(implicants.begin() + k);
            }
        }
        for(int i = 0; i < implicants.size(); ++i){
            std::cout << getLettersFromImplicants(implicants[i].simplified_number, input_size);
            if(i != implicants.size() - 1)
                std::cout << " + ";
        }
    }
    return -1;
}

MasterTable* groupByBits(MasterTable &master){
    auto* tmp_master_table = new MasterTable(master.size());
    Prime_Implicant tmp_implicant;
    int position;
    //Index Groups
    for(int i = 0; i < master.size() - 1; ++i){
        //Index Minterms for i
        for(int j = 0; j < master[i].size(); ++j){
            //Index Minterms for i + 1
            for(int k = 0; k < master[i + 1].size(); ++k){
                position = getUniqueChangePosition(master[i][j].simplified_number, master[i + 1][k].simplified_number);
                if(position != -1) {
                    tmp_implicant.minterms.insert(tmp_implicant.minterms.end(), master[i][j].minterms.begin(), master[i][j].minterms.end());
                    tmp_implicant.minterms.insert(tmp_implicant.minterms.end(), master[i + 1][k].minterms.begin(), master[i + 1][k].minterms.end());
                    tmp_implicant.simplified_number = setDontCareBit(master[i][j].simplified_number, position);
                    tmp_implicant.was_combined = false;
                    master[i][j].was_combined = true;
                    master[i + 1][k].was_combined = true;
                    if(!existInTable((*tmp_master_table)[i], tmp_implicant.simplified_number))
                        (*tmp_master_table)[i].push_back(tmp_implicant);
                    tmp_implicant.minterms.clear();
                }
            }
        }
    }

    for(auto& i : master)
        for(auto& j : i)
            if(!j.was_combined)
                (*tmp_master_table)[countBits(j.simplified_number)].push_back(j);
    return tmp_master_table;
}

bool existInTable(BitGroup & table, std::string& data){
    for(auto& i : table)
        if(i.simplified_number == data)
            return true;
    return false;
}

BitGroup makeImplicantTable(MasterTable& master, NumberList& minterms_list,ImplicantTable& implicant_table){
    //Initialize implicant table
    int position = 0;
    BitGroup prime_implicants, essential_prime_implicants;
    getImplicantFromMaster(master, prime_implicants);
    std::vector<bool> tmp_implicant_row(prime_implicants.size(), false);
    for(int i = 0; i < minterms_list.size(); ++i){
        implicant_table.push_back(tmp_implicant_row);
    }
    //Fills with prime implicants
    for(int i = 0; i < prime_implicants.size(); ++i)
        for(auto& j : prime_implicants[i].minterms)
            implicant_table[findFirstMintermFromList(minterms_list, j)][i] = true;
    //Analyze rows
    int unique_truth = -1;
    bool was_changed = false;
    for(auto & i : implicant_table){
        for(int j = 0; j < i.size(); ++j){
            if(i[j]){
                if(!was_changed){
                    unique_truth = j;
                    was_changed = true;
                }
                else{
                    unique_truth = -1;
                    break;
                }
            }
        }
        if(unique_truth != -1)
            essential_prime_implicants.push_back(prime_implicants[unique_truth]);
        was_changed = false;
    }
    //Deletes all minterms reached by essential prime implicants
    for(int i = 0; i < essential_prime_implicants.size(); ++i){
        for(int j = 0; j < essential_prime_implicants[i].minterms.size(); ++j){
            position = findFirstMintermFromList(minterms_list, essential_prime_implicants[i].minterms[j]);
            if(position != -1)
                minterms_list.erase(minterms_list.begin() + position);
        }
    }
    //Match for residual essential minterms
    NumberList points(prime_implicants.size(), 0);
    if(minterms_list.size() != 0){
        for(int i = 0; i < minterms_list.size(); ++i){
            for(int j = 0; j < prime_implicants.size(); ++j)
                for(int k = 0; k < prime_implicants[j].minterms.size(); ++k)
                    points[j] += (prime_implicants[j].minterms[k] == minterms_list[i]) ? 1 : 0;
        }
        //Push residual essenstial minterms
        for(int j = 0; j < points.size(); ++j)
            if(points[j] == minterms_list.size())
                essential_prime_implicants.push_back(prime_implicants[j]);
    }
    return essential_prime_implicants;
}

void getImplicantFromMaster(MasterTable& master, BitGroup& prime_implicants_list){
    for(auto& i : master)
        for(auto& j : i)
            prime_implicants_list.push_back(j);
}

bool isSameTable(MasterTable& table, MasterTable* tmp_master){
    try{
        for(int i = 0; i < table.size(); ++i){
            for(int j = 0; j < table[i].size(); ++j)
                if(table[i][j].simplified_number != (*tmp_master)[i][j].simplified_number)
                    return false;
        }
        return true;
    }catch(std::out_of_range& e){
        return false;
    }
}

ushort countBits(std::string& number){
    ushort bit_count = 0;
    for(auto& i : number)
        bit_count += (i == '1');
    return bit_count;
}

std::string numberToExtendedBinaryString(ushort number, ushort input_size){
    std::string binary_expression;
    setBinaryExpressionZeros(binary_expression, input_size);
    while(number != 0){
        binary_expression[(input_size--) - 1] = static_cast<char>(48 + (number % 2));
        number /= 2;
    }
    return binary_expression;
}

void setBinaryExpressionZeros(std::string& binary, ushort input_size){
    while(input_size-- > 0)
        binary += '0';
}

int getUniqueChangePosition(std::string& binary_expression1, std::string& binary_expression2){
    int position = 0;
    bool was_changed_before = false;
    for(int i = 0; i < binary_expression1.size(); ++i)
        if(binary_expression1[i] != binary_expression2[i]){
            if (!was_changed_before) {
                position = i;
                was_changed_before = true;
            }
            else
                return -1;
        }
    return position;
}

std::string setDontCareBit(std::string binary_expression, unsigned short position){
    binary_expression[position] = '-';
    return binary_expression;
}

int findFirstMintermFromList(NumberList& minterms, ushort minterm){
    for(int i = 0; i < minterms.size(); ++i)
        if(minterms[i] == minterm)
            return i;
    return -1;
}

void printImplicantTable(ImplicantTable & table){
    for(auto& i : table){
        for(auto && j : i)
            std::cout << j << " ";
        std::cout << std::endl;
    }

}

std::string getLettersFromImplicants(std::string& binary_expression, ushort variable_number){
    std::string products;
    for(int i = 0; i < binary_expression.size(); ++i){
        if(binary_expression[i] != '-'){
            products += static_cast<char>(65 + i);
            if(binary_expression[i] == '0')
                products += '\'';
        }
    }
    return products;
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

bool entradaRangoValido(int numero, size_t maximo){
    if(numero <= ERROR_LECTURA || numero >= maximo){
        std::cout << "Intente con valor numerico positivo mayor a 0 y menor a " << maximo << std::endl;
        return false;
    }
    return true;
}

void obtenerMinterminos(NumberList & lista, const unsigned short numero_variables){
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

void eliminarMinterminos(NumberList & lista, const unsigned short numero_variables){
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

void imprimirMinterminos(NumberList & lista){
    std::cout << "Lista minterminos: " << std::endl;
    for(auto& i : lista)
        std::cout << i << " ";
    std::cout << std::endl;
}

void menuPrincipal(NumberList& minterms, ushort input_size, MasterTable& table){
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
                    obtenerMinterminos(minterms, input_size);
                    break;
                case 1:
                    eliminarMinterminos(minterms, input_size);
                    break;
                case 2:
                    opcion = initReduction(minterms, input_size, table);
                    break;
                default:
                    break;
            }
    }while(opcion != SALIR_MENU);
}
