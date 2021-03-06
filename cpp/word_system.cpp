#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <iterator>
#include <utility>
#include <set>
#include <map>


using namespace std;


typedef pair<short, short> location_pair;
typedef vector<location_pair> location;
typedef vector<string> text;
typedef pair<text*, location*> text_location;
typedef map<string, location*> text_map;
typedef text_map::value_type tmvt;
/* 单词排除集合, 嗝屁的 */
typedef set<string>::difference_type diff_type;
set<string> exclusion_set;


void suffix_s(string word){
    /**
     *  babies, ius, s
    **/
    string::size_type pos3 = word.size() - 3;
    string ies("ies");
    if(!word.compare(pos3, 3, ies)){
        // 10种重载方式......
        word.replace(pos3, 3, 1, 'y');
        return;
    }

    string ses("ses");
    if(!word.compare(pos3, 3, ses)){
        // 只要删除es即可
        word.erase(pos3+1, 2);
    }

    string::size_type spos = 0;
    string suffixs("oussisius");
    if(!word.compare(pos3, 3, suffixs, spos, 3) || !word.compare(pos3, 3, suffixs, spos+6, 3) ||
        !word.compare(pos3+1, 2, suffixs, spos+4, 2) || !word.compare(pos3+1, 2, suffixs, spos+2, 2)){
            return;
    }
}


void suffix_text(vector<string>* words){
    /**
     * 去掉以s结尾的单词
    **/
    vector<string>::iterator iter_begin = words->begin();
    vector<string>::iterator iter_end = words->end();
    while(iter_begin != iter_end){
        if((*iter_begin).size() <= 3){
            /* 长度小于3的单词忽略 */
            iter_begin ++;
        }
        else if((*iter_begin)[(*iter_begin).size()-1] == 's'){
            /* 以s结尾 */
            suffix_s(*iter_begin);
            iter_begin ++;
        }
    }
}


void strip_cap(vector<string>* words){
    /**
     * 去除文本中, 所有的大写字母, 全市小写的单词
    **/
    vector<string>::iterator iter_begin = words->begin();
    vector<string>::iterator iter_end = words->end();

    string caps("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    while(iter_begin != iter_end){
        string::size_type pos=0;
        while((pos=(*iter_begin).find_first_of(caps, pos)) != string::npos){
            (*iter_begin)[pos] = tolower((*iter_begin)[pos]);
        }
        iter_begin++;
    }
}


void filter_text(vector<string>* words, string filter){
    /**
     * 去掉单词中不需要的字符, ",.?()等
    **/
    vector<string>::iterator iter_begin = words->begin();
    /* 在遍历的时候，对原数据做了修改，所以把end()先提出来 */
    vector<string>::iterator iter_end = words->end();
    if(!filter.size()){
       filter.insert(0, "\".,");
    }
    while(iter_begin != iter_end){
       string::size_type pos=0;
       while((pos=(*iter_begin).find_first_of(filter, pos)) != string::npos){
           /**
            * erase(pos, n);
            * erase(position); position 是string的迭代器
            * erase(first, second); 从first- second删除
           **/
           (*iter_begin).erase(pos, 1);
       }
       iter_begin++;
    }
}


text_location* separate_words(const vector<string>* text_file){
    /**
     * words 包含独立的单词集合
     * locations 包含单词的行列信息
    **/
    vector<string>*  words = new vector<string>;
    vector<location_pair>* locations = new vector<location_pair>;
    short line_num = 0;
    for(; line_num < text_file->size(); line_num++){
        short word_pos;
        string text_line = (*text_file)[line_num];
        string::size_type pos=0, pre_pos=0;
        while((pos=text_line.find_first_of(' ', pos)) != string::npos){
            words->push_back(text_line.substr(pre_pos, pos-pre_pos));
            locations->push_back(location_pair(line_num, word_pos));
            ++word_pos;
            pre_pos = ++pos;
        }
        words->push_back(text_line.substr(pre_pos, pos-pre_pos));
        locations->push_back(location_pair(line_num, word_pos));
        filter_text(words, "\",.;:!?)(\\/");
        strip_cap(words);    //忽略大小写
    }
    return new text_location(words, locations);
}

extern text_map* build_word_map(const text_location* text_locations){
    /**
     * 使用word_map[key] key不存在, 则插入一个对象, value是缺省的默认值
    */
    text_map* word_map = new text_map();
    vector<string>* text_words = text_locations->first;
    vector<location_pair>* text_locs = text_locations->second;
    int elem_cnt = text_words->size();
    for( int i=0; i<elem_cnt; i++){
        /**
         * 单词的长度和是否存在
        */
       string word = (*text_words)[i];
       /* 因为我的数据都是1-9所以，加这个条件会导致没，没有数据输出.
       if(word.size() < 3 || exclusion_set.count(word)){
           continue;
       }*/
       if(!word_map->count((*text_words)[i])){
           /**/
           location* loc = new location();
           loc->push_back((*text_locs)[i]);
           word_map->insert(tmvt(word, loc));
       }
       else{
           (*word_map)[word]->push_back((*text_locs)[i]);
       }
    }
    return word_map;
}


void display_map_text(text_map* word_map){
    text_map::iterator iter = word_map->begin();
    while(iter != word_map->end()){
        cout << iter->first << ": ";
        location::iterator loc_iter = iter->second->begin();
        while(loc_iter != iter->second->end()){
            cout << "<" << loc_iter->first << ", " << loc_iter->second << "> ";
            loc_iter ++;
        }
        cout << "\n";
        iter++;
    }
}


vector<string>* retrieve_text(){
    /**
     * 主要学会文件的读写
    **/
    string file_name;
    cout << "input file name: "; cin >> file_name;
    ifstream infile(file_name.c_str(), ios::in);
    if(!infile){
        cout << "unable to open file" << endl;
        exit(-1);
    }
    vector<string>* line_of_text = new vector<string>;
    vector<string> words;
    string text_line;
    typedef pair<string::size_type, int> stats;
    stats maxline;
    int line_num = 0;
    while(getline(infile, text_line, '\n')){
        cout << "line read: " << text_line << '\n';
        if(maxline.first << text_line.size()){
            maxline.first = text_line.size();
            maxline.second = line_num;
        }
        line_of_text->push_back(text_line);
        line_num++;

        string::size_type pos=0, prev_pos=0;
        while((pos=text_line.find_first_of(' ', pos)) != string::npos){
            words.push_back(text_line.substr(prev_pos, pos-prev_pos));
            prev_pos = ++pos;
        }
    }
    return line_of_text;
}


void get_exclusion_set(){
    ifstream infile("exclusion_set.txt");
    if(!infile){
        static string default_excluded_words[25] = {
            "the","and","but","that","then","are","been",
            "can","can't","cannot","could","did","for",
            "had","have","him","his","her","its","into",
            "were","which","when","with","would"
        };
        cerr << "Unable to open file exclusion_set.txt, Use default exslusion" << endl;
        copy(default_excluded_words, default_excluded_words+25, 
             inserter(exclusion_set, exclusion_set.begin()));
    }
    else{
        istream_iterator<string, diff_type> input_set(infile), eof;
        copy(input_set, eof, inserter(exclusion_set, exclusion_set.begin()));
    }
}


void print(vector<string>* svec){
    vector<string>::const_iterator iter = svec->begin();
    while(iter != svec->end()){
        cout << *iter << endl;
        iter++;
    }
}


int main(){
    vector<string>* res = retrieve_text();
    print(res);
    text_location *text_locations = separate_words(res);
    text* text_locations_text = text_locations->first;
    location* text_locations_locations = text_locations->second;
    int len = text_locations_text->size();
    for( int index=0; index<len; index++){
        cout << "Word: " << (*text_locations_text)[index] << " Location: ("
             << (*text_locations_locations)[index].first << ", "
             << (*text_locations_locations)[index].second << ")" << endl;
    }
    text_map* text_map_res = build_word_map(text_locations);
    display_map_text(text_map_res);
    return 0;
}