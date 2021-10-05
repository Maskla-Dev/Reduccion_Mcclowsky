/*
 * Luis Antonio Montoya Morales
 * An assigment for Fundamentos de Programacion
 * Last Update October, 5th, 2021
 */

#include <iostream>
#include <vector>
#include <string>

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

using ushort = unsigned short;
using NumberList = std::vector<ushort>;         //Index NumberList

struct Prime_Implicant{
    NumberList minterms;
    std::string simplified_number;
};

using BitGroup = std::vector<Prime_Implicant>;  //Index Prime Implicant
using MasterTable = std::vector <BitGroup>;     //Index BitGroups
using ImplicantTable = std::vector<std::vector<bool>>; //Bi-dimensional array

MasterTable* groupByBits(MasterTable&);
ushort countBits(std::string&);
bool isEmptyTable(MasterTable&);
std::string numberToExtendedBinaryString(ushort, ushort);
void setBinaryExpressionZeros(std::string&, ushort);
int getUniqueChangePosition(std::string&, std::string&);
std::string setDontCareBit(std::string, unsigned short);
void cleanMaster(MasterTable&);
void getImplicantFromMaster(MasterTable&, BitGroup&);
BitGroup makeImplicantTable(MasterTable&, NumberList&,ImplicantTable&);
ushort findFirstMintermFromList(NumberList&, ushort);
void printImplicantTable(ImplicantTable&);
std::string getLettersFromImplicants(std::string&, ushort);

int main(){
    ushort input_size = 4;
    NumberList minterms({1,3,4,5,9,11,12,13,14,15});
    MasterTable table(input_size), *tmp_table;
    Prime_Implicant tmp;
    ImplicantTable implicant_table;
    BitGroup essential_implicants;
    //First Iteration
    for(auto& i : minterms){
        tmp.minterms.push_back(i);
        tmp.simplified_number = numberToExtendedBinaryString(i, input_size);
        table[countBits(tmp.simplified_number) - 1].push_back(tmp);
        tmp.minterms.clear();
    }
    //Second Iteration
    while(table.size() >= 2){
        tmp_table = groupByBits(table);
        if(!isEmptyTable(*tmp_table)){
            table = *tmp_table;
            delete tmp_table;
            tmp_table = nullptr;
        }
        else
            break;
    }
    cleanMaster(table);
    essential_implicants = makeImplicantTable(table, minterms, implicant_table);
    if(!essential_implicants.empty()){
        for(int i = 0; i < essential_implicants.size(); ++i){
            std::cout << getLettersFromImplicants(essential_implicants[i].simplified_number, input_size);
            if(i != essential_implicants.size() - 1)
                std::cout << " + ";
        }
    }
    else{
        for(int i = 0; i < table.size(); ++i){
            for(int j = 0; j < table[i].size(); ++j)
                std::cout << getLettersFromImplicants(table[i][j].simplified_number, input_size);
            if(i != essential_implicants.size() - 1)
                std::cout << " + ";
        }
    }
    return 0;
}

MasterTable* groupByBits(MasterTable &master){
    auto* tmp_master_table = new MasterTable(master.size() - 1);
    Prime_Implicant tmp_implicant;
    int position;
    //Index Groups
    for(int i = 0; i < master.size() - 1; ++i){
        //Index Minterms for i
        for(int j = 0; j < master[i].size(); ++j){
            //Index Minterms for i + 1
            for(int k = 0; k < master[i + 1].size(); ++k){
                //Count bits for ith prime implicant and the next XOR operation, if counts 1 merge and simplify with XOR operation
                position = getUniqueChangePosition(master[i][j].simplified_number, master[i + 1][k].simplified_number);
                if(position != -1) {
                    tmp_implicant.minterms.insert(tmp_implicant.minterms.end(), master[i][j].minterms.begin(), master[i][j].minterms.end());
                    tmp_implicant.minterms.insert(tmp_implicant.minterms.end(), master[i + 1][k].minterms.begin(), master[i + 1][k].minterms.end());
                    tmp_implicant.simplified_number = setDontCareBit(master[i][j].simplified_number, position);
                    (*tmp_master_table)[i].push_back(tmp_implicant);
                    tmp_implicant.minterms.clear();
                }
            }
        }
    }
    return tmp_master_table;
}

BitGroup makeImplicantTable(MasterTable& master, NumberList& minterms_list,ImplicantTable& implicant_table){
    //Initialize implicant table
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
    return essential_prime_implicants;
}

void getImplicantFromMaster(MasterTable& master, BitGroup& prime_implicants_list){
    for(auto& i : master)
        for(auto& j : i)
            prime_implicants_list.push_back(j);
}

bool isEmptyTable(MasterTable& table){
    bool isEmpty = false;
    if(table.size() == 1)
        return true;
    else{
        for(int i = 0; i < table.size() - 1; ++i)
            isEmpty = table[i].empty() && table[i + 1].empty();
        return isEmpty;
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

void cleanMaster(MasterTable& master){
    for(auto& i : master){
        //Index Minterms for i
        for(int j = 0; j < i.size() - 1; ++j){
            for(int k = j + 1; k < i.size(); ++k){
                if(i[j].simplified_number == i[k].simplified_number)
                    i.erase(i.begin() + k);
            }
        }
    }
}

ushort findFirstMintermFromList(NumberList& minterms, ushort minterm){
    for(int i = 0; i < minterms.size(); ++i)
        if(minterms[i] == minterm)
            return i;
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