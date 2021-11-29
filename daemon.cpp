//https://medium.com/@syedaarzoo99_58616/simple-diff-utility-using-dynamic-programming-77c80bd87f5d
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <deque>

bool    isi = false,
        isE = false,
        isZ = false,
        isb = false,
        isw = false,
        isB = false,
        ist = false;

const uint32_t TABS_COUNT = 7;

inline void get_lines(
    std::ifstream& file,
    std::deque<std::string>& lines,
    std::deque<std::string>& lines_orig){
    std::string tmp;
    while(std::getline(file, tmp)){
        if(ist)
            for(uint32_t i = 0; i < tmp.size(); ++i)
                if(tmp[i] == '\t'){
                    tmp[i] = ' ';
                    for(uint32_t tmpi = 0; tmpi < TABS_COUNT - 1; ++tmpi)
                        tmp.insert(tmp.begin() + i, ' ');
                    i += TABS_COUNT - 2;
                }
    
        lines_orig.push_back(tmp);
        if(isB && tmp.empty())
            continue;
        int space_count = 0;
        for(uint32_t i = 0; i < tmp.size(); ++i){
            if(isi && tmp[i] >= 'A' && tmp[i] <= 'Z')
                tmp[i] = std::tolower(tmp[i]);

            if(isE && tmp[i] == '\t')
                tmp[i] = ' ';



            if(tmp[i] == ' ')
                if(isw){
                    tmp.erase(tmp.begin() + i);
                    --i;
                    space_count = 0;
                }
                else if(isb && space_count){
                    tmp.erase(tmp.begin() + i);
                    --i;
                }
                else
                    ++space_count;
            else
                space_count = 0;
        }
        
        if(isZ && tmp[tmp.size() - 1] == ' ')
            tmp.pop_back();
        lines.push_back(tmp);
    }
}

// For debugging
#define   print     \
    for(uint32_t i = 0; i < matrix[0].size(); ++i) \
        if(i == 0)  \
            std::cout << "  / "; \
        else \
            std::cout << first_lines[i - 1] << ' '; \
    std::cout << '\n'; \
    for(uint32_t i = 0; i < matrix.size(); ++i){ \
        if(i == 0){ \
            std::cout << "/ "; \
            for(uint32_t j = 0; j < matrix[0].size(); ++j) \
                std::cout << matrix[0][j] << ' '; \
            std::cout << '\n'; \
            continue; \
        } \
        std::cout << second_lines[i - 1] << ' '; \
        for(uint32_t j = 0; j < matrix[0].size(); ++j) \
            std::cout << matrix[i][j] << ' '; \
        std::cout << '\n'; \
    } 


int main(int argc, char **argv){
    std::ifstream first_file, second_file;
    bool is_checked_first = false, is_checked_second = false;

    // Parse arguments
    for(int i = 1; i < argc; ++i)
        if(argv[i][0] == '-'){
            for(uint32_t j = 1; j < strlen(argv[i]); ++j)
                switch (argv[i][j]){
                    case 'i':
                        isi = true;
                        break;
                    case 'E':
                        isE = true;
                        break;
                    case 'Z':
                        isZ = true;
                        break;
                    case 'b':
                        isb = true;
                        break;
                    case 'w':
                        isw = true;
                        break;
                    case 'B':
                        isB = true;
                        break;
                    case 't':
                        ist = true;
                        break;
                    default:
                        std::cout << "diffda: " << argv[i][j] << ": Неизвестный флаг!\n";
                        return 1;
                }
        }
        else if(!is_checked_first){
            first_file.open(argv[i]);
            is_checked_first = true;
        }
        else if(!is_checked_second){
            second_file.open(argv[i]);
            is_checked_second = true;
        }
        else{
            std::cout << "diffda: " << argv[i] << ": Нельзя сранивать больше двух файлов!\n";
            return 1;
        }

    // Checking files for good opening
    bool check_exit = false;
    if(!first_file.good()){
        std::cout << "diffda: " << argv[1] << ": Нет такого файла или каталога!\n";
        check_exit = true;
    }
    if(!second_file.good()){
        std::cout << "diffda: " << argv[2] << ": Нет такого файла или каталога!\n";
        check_exit = true;
    }
    
    // Exit if check was bad
    if(check_exit)
        return 1;

    // Parsed lines after flags
    std::deque<std::string> first_lines, second_lines;

    // Clear lines(or with -t) to output
    std::deque<std::string> first_lines_original, second_lines_original;

    // Read files and convert to data
    std::string tmp;
    get_lines(first_file, first_lines, first_lines_original);
    get_lines(second_file, second_lines, second_lines_original);

    // Calculate matrix to get output
    std::deque<std::deque<int>> matrix(second_lines.size() + 1, std::deque<int>(first_lines.size() + 1));
    for(uint32_t i = 1; i < matrix.size(); ++i)
        for(uint32_t j = 1; j < matrix[0].size(); ++j)
            if(first_lines[j - 1] == second_lines[i - 1])
                matrix[i][j] = matrix[i - 1][j - 1] + 1;
            else
                matrix[i][j] = std::max(matrix[i - 1][j], matrix[i][j - 1]);

    // Struct to make output more comfortable
    struct t_result_item{
        // 'c' - change
        // 'a' - add
        // 'd' - delete
        char type;
        // for 'c' its range
        // for others type is using only start which describes index
        uint32_t start, end;
        // Index from other file
        uint32_t index_from_other_file;
    };

    // Output
    std::deque<t_result_item> result;

    uint32_t i = matrix.size() - 1, j = matrix[0].size() - 1;
    // 'u' - up
    // 'l' - left
    // '\0' - diagonal or start char
    char prev_type = '\0';
    // Parse matrix
    while(i > 0 || j > 0){
        if(i == 0){
            if(prev_type == 'c' || prev_type == 'l')
                result[result.size() - 1].start = --j;
            else if(prev_type == '\0'){
                result.push_back({'l', --j, j, i});
                prev_type = 'l';
            }
            else{
                result.push_back({'c', --j, j});
                prev_type = 'c';
            }
            continue;
        }
        else if(j == 0){
            if(prev_type == 'c' || prev_type == 'u')
                result[result.size() - 1].start = --i;
            else if(prev_type == '\0'){
                result.push_back({'u', --i, i, j});
                prev_type = 'u';
            }
            else{
                result.push_back({'c', --i, i});
                prev_type = 'c';
            }
            continue;

        }

        // Is up == left and char up == char left, so move diagonally
        if(matrix[i - 1][j] == matrix[i][j - 1] &&
            first_lines[j - 1] == second_lines[i - 1]){
            --i, --j;
            prev_type = '\0';
        }
        // Is up >= left
        else if(matrix[i - 1][j] >= matrix[i][j - 1])
            if(prev_type == 'u' || prev_type == 'c')
                result[result.size() - 1].start = --i;
            else if(prev_type == 'l'){
                result.push_back({'c', --i, i});
                prev_type = 'c';
            }
            else{
                result.push_back({'u', --i, i, j});
                prev_type = 'u';
            }
        // Is left > up
        else
            if(prev_type == 'l' || prev_type == 'c')
                result[result.size() - 1].start = --j;
            else if(prev_type == 'u'){
                result.push_back({'c', --j, j});
                prev_type = 'c';
            }
            else{
                result.push_back({'l', --j, j, i});
                prev_type = 'l';
            }   
    }

    // Debugging
    // for(const auto& item: result)
    //     std::cout << " " << item.start + 1 << ' ' << item.end + 1 << " : " <<
    //         item.type << ' ' << item.index_from_other_file << '\n';
    // print

    // Parse output
    for(auto it = result.rbegin(); it != result.rend(); ++it){
        if(it->type == 'u'){
            std::cout << it->index_from_other_file << 'a' << (it->start == it->end 
                ? (std::to_string(it->start + 1) + '\n') 
                : (std::to_string(it->start + 1) + ',' + std::to_string(it->end + 1) + '\n'));
            for(uint32_t i = it->start; i <= it->end; ++i)
                std::cout << "> " << second_lines_original[i] << '\n';
        }
        else if(it->type == 'l'){
            std::cout << ((it->start == it->end) ? std::to_string(it->start + 1)
                : (std::to_string(it->start + 1) + ',' + std::to_string(it->end + 1)))
                << 'd' << it->index_from_other_file << '\n';
            for(uint32_t i = it->start; i <= it->end; ++i)
                std::cout << "< " << first_lines_original[i] << '\n';
        }
        else{
            auto it2 = it + 1;
            if(it2->type == 'l'){
                std::cout << ((it2->start == it2->end) 
                    ? (std::to_string(it2->start + 1))
                    : (std::to_string(it2->start + 1) + ',' + std::to_string(it2->end + 1)))
                    << 'c' << ((it->start == it->end) 
                    ? (std::to_string(it->start + 1))
                    : (std::to_string(it->start + 1) + ',' + std::to_string(it->end + 1)))
                    << '\n';
                for(uint32_t i = it2->start; i <= it2->end; ++i)
                    std::cout << "< " << second_lines_original[i] << '\n';
                std::cout << "---\n";
                for(uint32_t i = it->start; i <= it->end; ++i)
                    std::cout << "> " << first_lines_original[i] << '\n';
            }
            else{
                std::cout << ((it->start == it->end) 
                    ? (std::to_string(it->start + 1))
                    : (std::to_string(it->start + 1) + ',' + std::to_string(it->end + 1)))
                    << 'c' << ((it2->start == it2->end) 
                    ? (std::to_string(it2->start + 1))
                    : (std::to_string(it2->start + 1) + ',' + std::to_string(it2->end + 1)))
                    << '\n';
                for(uint32_t i = it->start; i <= it->end; ++i)
                    std::cout << "< " << first_lines_original[i] << '\n';
                std::cout << "---\n";
                for(uint32_t i = it2->start; i <= it2->end; ++i)
                    std::cout << "> " << second_lines_original[i] << '\n';
            }
            ++it;
        }
    }
    return 0;
}
// https://medium.com/@syedaarzoo99_58616/simple-diff-utility-using-dynamic-programming-77c80bd87f5d
// https://www.geeksforgeeks.org/longest-common-subsequence-dp-4/