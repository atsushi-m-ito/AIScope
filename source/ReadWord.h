#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


class ReadWord{
private:
    FILE* m_fp;
    const int m_line_size;
    char* m_line;
    const char* m_delimiter;
    const char* m_singles;

    char* m_last_pointer;
    char m_last_end_char;
public:
    
    /*
    delimiter: strtokのdelimiterと同じ
    singles: 他の単語とは連結しない単独文字,delimiterにもなるが無視はされない.
    */
    ReadWord(FILE* fp, const int line_size, const char* delimiter, const char* singles) :
        m_fp(fp),
        m_line_size(line_size),
        m_line(new char[line_size]),
        m_delimiter(delimiter),
        m_singles(singles),
        m_last_end_char('\0')
    {
        m_last_pointer = m_line;
        m_line[0] = '\0';

    };

    ~ReadWord(){
        delete[] m_line;

    };

    char* Get(){

        //前回の読み込みで'\0'で埋めた部分を戻す//
        *m_last_pointer = m_last_end_char;
        char* p = m_last_pointer;
        char* head = NULL;

        //単語の先頭さがし//
        while (1){
            if (*p == '\0'){        //読み込んだ行の末尾まで来ている//


                //新しい行を読み込み//
                if (fgets(m_line, m_line_size, m_fp) == NULL){
                    //もうファイルが無い//
                    return NULL;
                }

                //読み込み成功//
                p = m_line;
            

            } else if (strchr(m_delimiter, *p) != NULL){
                //delimitterだったので次の文字へ//
                p++;
            
            }else{  //単語の先頭が見つかった//

                //独立文字だった為次の文字を記憶して終了//
                if (strchr(m_singles, *p) != NULL){
                    
                    m_last_pointer = p + 1;
                    m_last_end_char = *m_last_pointer;
                    *m_last_pointer = '\0';
                    return p;
                
                //独立文字ではなかったので、単語の末尾を探すルートへ進む//
                } else{
                    head = p;
                    p++;
                    break;
                }
            }
        }


        //単語の末尾を探す//
        while (1){
            if (*p == '\0'){        //読み込んだ行の末尾まで来た//
                //末尾が見つかったことと同義//
                m_last_pointer = p;
                m_last_end_char = *m_last_pointer;
                return head;
                break;

            } else if (strchr(m_delimiter, *p) != NULL){
                //delimitterだった. 末尾が見つかったことと同義//
                m_last_pointer = p;
                m_last_end_char = *m_last_pointer;
                *p = '\0';
                return head;

            } else if (strchr(m_singles, *p) != NULL){
                //独立文字だった為次の文字を記憶して終了//
                m_last_pointer = p;
                m_last_end_char = *m_last_pointer;
                *p = '\0';
                return head;

                //単語末尾ではなかったので次の文字へ//
            } else{
                p++;

            }
        }
        
        //ここには来ないはず
        return NULL;

    }
};

