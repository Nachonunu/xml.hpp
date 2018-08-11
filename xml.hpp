/*! xml.hpp v0.1 | CC0 1.0 (https://creativecommons.org/publicdomain/zero/1.0/deed) */

#ifndef _XML_HPP
#define _XML_HPP

#include <map>
#include <string>
#include <iostream>
using namespace std;

template <typename T> class xml_node;
template <typename T> class xml_attribute;
template <typename T> class xml_document;

enum xml_obj_type {
	none,
	attribute,
	node
};

enum xml_node_type {
	document, //名前なし 属性なし 子ノードあり xmlドキュメントのトップ
	doctype,  //名前なし 属性なし 子ノードなし 
	comment,  //名前なし 属性なし 子ノードなし 
	normal,   //名前あり 属性あり 子ノードあり 通常のノード
	pi        //名前なし 属性なし 子ノードなし 
};

template <typename T = char>
class xml_object {
protected:
	string o_name;
	string o_value;
	xml_node<T>* n_parent;
public:
	xml_object() {}
	virtual ~xml_object() {}

	virtual xml_obj_type obj_type() { return none; }
	xml_node<T>* parent() { return n_parent; }
	virtual string name() { return o_name; }
	virtual void name(const string& text) { o_name = text; }
	virtual string value() { return o_value; }
	virtual void value(const string& text) { o_value = text; }
};

template <typename T = char>
class xml_attribute : public xml_object<T> {
	using xml_object<T>::o_name;
	using xml_object<T>::o_value;
	using xml_object<T>::n_parent;

	void set_parent() { //親ノードに自分を登録
		xml_attribute<T>* attr = n_parent->find_attribute(o_name);
		if (attr != nullptr && attr != this) {
			//既に別の属性が存在する場合それを削除
			attr->n_parent = nullptr;
			delete attr;
		}
		n_parent->a_list[o_name] = this;
	}

	friend class xml_node<T>;
public:
	xml_attribute() { n_parent = nullptr; }
	xml_attribute(const string& n, const string& v) { o_name = n; o_value = v; n_parent = nullptr; }
	xml_attribute(const string& n, const string& v, xml_node<T>* parent) { //親ノードに自分を登録
		o_name = n;
		o_value = v;
		n_parent = parent;
		set_parent();
	}
	~xml_attribute() { clear_parent(); }

	xml_obj_type obj_type() { return attribute; }

	void clear_parent() {
		if (n_parent != nullptr) {
			//親ノードの登録を削除
			n_parent->a_list.erase(o_name);
			n_parent = nullptr;
		}
	}

	void move(xml_node<T>* node) {
		clear_parent();
		if (node != nullptr) {
			n_parent = node;
			set_parent();
		}
	}

	string name() { return o_name; }
	void name(const string& text) {
		if (n_parent == nullptr) o_name = text;
		else {
			//現在の登録を削除
			n_parent->a_list.erase(o_name);
			o_name = text;
			set_parent();
		}
	}

	string value() { return o_value; }
	void value(const string& text) { o_value = text; }
};

template <typename T = char>
class xml_node : public xml_object<T> {
	using xml_object<T>::o_name;
	using xml_object<T>::o_value;
	using xml_object<T>::n_parent;

	xml_node_type n_type;
	map<string, xml_attribute<T>*> a_list;

	xml_node<T>* n_back;
	xml_node<T>* n_next;
	xml_node<T>* n_first;
	xml_node<T>* n_end;

	friend class xml_attribute<T>;
	friend class xml_document<T>;
public:
	xml_node() { n_back = nullptr; n_next = nullptr; n_parent = nullptr; n_first = nullptr; n_end = nullptr; n_type = normal; }
	xml_node(xml_node_type t) { n_back = nullptr; n_next = nullptr; n_parent = nullptr; n_first = nullptr; n_end = nullptr; n_type = t; }
	xml_node(const string& n, const string& v) {
		o_name = n;
		o_value = v;
		n_type = normal;
		n_back = nullptr;
		n_next = nullptr;
		n_parent = nullptr;
		n_first = nullptr;
		n_end = nullptr;
	}
	xml_node(const string& n, const string& v, xml_node_type t, xml_node<T>* node, int flag = 1) {
		n_type = t;
		switch (t) {
			case comment: case doctype: case pi:
				o_value = v;
				break;
			case normal:
				o_name = n;
				o_value = v;
				break;
			default: break;
		}
		n_first = nullptr;
		n_end = nullptr;
		if (flag <= 1) {
			//nodeを親ノードとして登録
			n_parent = node;
			if (n_parent->n_first == nullptr) {
				//親ノードに登録しているノードが一つもない
				n_parent->n_first = this;
				n_parent->n_end = this;
				n_back = nullptr;
				n_next = nullptr;
			} else if (flag <= 0) {
				// -0 最前列に追加
				n_parent->n_first->n_back = this;
				n_back = nullptr;
				n_next = n_parent->n_first;
				n_parent->n_first = this;
			} else {
				// 1 最後尾に追加
				n_parent->n_end->n_next = this;
				n_back = n_parent->n_end;
				n_next = nullptr;
				n_parent->n_end = this;
			}
		} else if (node->n_parent != nullptr) {
			//nodeの前後に自分を配置
			n_parent = node->n_parent;
			if (flag == 2) {
				// 2 nodeの前に配置
				if (node->n_next == nullptr) {
					//最後尾に配置
					n_parent->n_end = this;
					node->n_next = this;
					n_back = node;
					n_next = nullptr;
				} else {
					n_back = node;
					n_next = node->n_next;
					n_back->n_next = this;
					n_next->n_back = this;
				}
			} else {
				// 3- nodeの後ろに配置
				if (node->n_back == nullptr) {
					//最前列に配置
					n_parent->n_first = this;
					node->n_back = this;
					n_back = nullptr;
					n_next = node;
				} else {
					n_back = node->n_back;
					n_next = node;
					n_back->n_next = this;
					n_next->n_back = this;
				}
			}
		}
	}
	~xml_node() { clear(); }

	void clear() {
		delete_all_children(); //子ノードをすべて削除
		delete_all_attribute(); //属性をすべて削除
		//親ノードとのリンクをクリア
		if (n_parent != nullptr) {
			if (n_parent->first_child() == this) n_parent->n_first = n_next;
			if (n_parent->end_child() == this) n_parent->n_end = n_back;
		}
		//前後のつながりを修正
		if (n_back != nullptr) {
			if (n_next != nullptr) {
				n_back->n_next = n_next;
				n_next->n_back = n_back;
			} else n_back->n_next = nullptr;
		} else if (n_next != nullptr) n_next->n_back = nullptr;
	}

	//取得・編集
	xml_node_type type() { return n_type; }
	void type(xml_node_type t) {
		n_type = t;
		if (t == document) delete_all_attribute();
		else if (t == comment || t == doctype) {
			delete_all_children();
			delete_all_attribute();
		}
	}
	xml_obj_type obj_type() { return node; }
	string name() { return o_name; }
	void name(const string& n) { if (n_type == normal || n_type == pi) o_name = n; }
	string value() { return o_value; }
	void value(const string& v) { if (n_type == normal || n_type == comment || n_type == doctype) o_value = v; }
	void add_value(const string& v) { if (n_type == normal || n_type == comment || n_type == doctype) o_value += v; }
	//子ノード取得
	xml_node<T>* first_child() {
		if (n_type == document || n_type == normal) return n_first; //子ノードが存在するタイプのみ
		return nullptr;
	}
	xml_node<T>* first_child(const string& n) {
		if (n_type == document || n_type == normal) { //子ノードが存在するタイプのみ
			//検索
			xml_node<T>* find_node = n_first;
			while (find_node != nullptr) {
				if (find_node->o_name == n) return find_node; //タグ名が一致するか判定
				find_node = find_node->n_next; //次のノード
			}
		}
		return nullptr;
	}
	xml_node<T>* end_child() {
		if (n_type == document || n_type == normal) return n_end; //子ノードが存在するタイプのみ
		return nullptr;
	}
	xml_node<T>* end_child(const string& n) {
		if (n_type == document || n_type == normal) { //子ノードが存在するタイプのみ
			//検索
			xml_node<T>* find_node = n_end;
			while (find_node != nullptr) {
				if (find_node->o_name == n) return find_node; //タグ名が一致するか判定
				find_node = find_node->n_back; //前のノード
			}
		}
		return nullptr;
	}
	//次のノード取得
	xml_node<T>* next_node() { return n_next; }
	xml_node<T>* next_node(const string& n) {
		xml_node<T>* find_node = n_next;
		while (find_node != nullptr) {
			if (find_node->o_name == n) return find_node; //タグ名が一致するか判定
			find_node = find_node->n_next; //次のノード
		}
		return nullptr;
	}
	//前のノード取得
	xml_node<T>* back_node() { return n_back; }
	xml_node<T>* back_node(const string& n) {
		xml_node<T>* find_node = n_back;
		while (find_node != nullptr) {
			if (find_node->o_name == n) return find_node; //タグ名が一致するか判定
			find_node = find_node->n_back; //前のノード
		}
		return nullptr;
	}
	//属性取得
	xml_attribute<T>* first_attribute() {
		if ((n_type == normal || n_type == pi) && !a_list.empty()) return a_list.begin()->second;
		return nullptr;
	}
	xml_attribute<T>* end_attribute() {
		if ((n_type == normal || n_type == pi) && !a_list.empty()) return a_list.end()->second;
		return nullptr;
	}
	xml_attribute<T>* find_attribute(const string& n) { //指定の属性を検索して取得
		if ((n_type == normal || n_type == pi) && !a_list.empty()) { //属性が存在
			auto it = a_list.find(n); //検索
			if (it != a_list.end()) return it->second; //指定の属性が存在
		}
		return nullptr;
	}
	//追加
	xml_node<T>* add_first_child(const string& n, const string& v, xml_node_type t = normal) {
		xml_node<T>* out = new xml_node<T>(n, v, t, this, 0);
		return out;
	}
	xml_node<T>* add_end_child(const string& n, const string& v, xml_node_type t = normal) {
		xml_node<T>* out = new xml_node<T>(n, v, t, this, 1);
		return out;
	}
	xml_node<T>* add_next_node(const string& n, const string& v, xml_node_type t = normal) {
		xml_node<T>* out = new xml_node<T>(n, v, t, this, 2);
		return out;
	}
	xml_node<T>* add_back_node(const string& n, const string& v, xml_node_type t = normal) {
		xml_node<T>* out = new xml_node<T>(n, v, t, this, 3);
		return out;
	}
	xml_attribute<T>* add_attribute(const string& n, const string& v) {
		if (n_type != normal && n_type != pi) return nullptr;
		xml_attribute<T>* attr = find_attribute(n);
		if (attr != nullptr) attr->value(v); //既に存在する属性に上書き
		else attr = new xml_attribute<T>(n, v, this); //新規作成
		return attr;
	}
	//ノード削除
	xml_node<T>* delete_first_child() {
		if (n_type != normal && n_type != document) return nullptr;
		xml_node<T>* out = nullptr;
		xml_node<T>* node = n_first;
		if (node->type() != document && node != nullptr) { out = node->next_node(); delete node; }
		return out;
	}
	xml_node<T>* delete_first_child(const string& n) {
		if (n_type != normal && n_type != document) return nullptr;
		xml_node<T>* out = nullptr;
		xml_node<T>* node = n_first;
		//検索
		while (node != nullptr) {
			if ((node->type() == normal || node->type() == pi) && node->name() == n) { out = node->next_node(); delete node; break; }
			node = node->next_node();
		}
		return out;
	}
	xml_node<T>* delete_end_child() {
		if (n_type != normal && n_type != document) return nullptr;
		xml_node<T>* out = nullptr;
		xml_node<T>* node = n_end;
		if (node->type() != document && node != nullptr) { out = node->back_node(); delete node; }
		return out;
	}
	xml_node<T>* delete_end_child(const string& n) {
		if (n_type != normal && n_type != document) return nullptr;
		xml_node<T>* out = nullptr;
		xml_node<T>* node = n_end;
		//検索
		while (node != nullptr) {
			if ((node->type() == normal || node->type() == pi) && node->name() == n) { out = node->back_node(); delete node; break; }
			node = node->back_node();
		}
		return out;
	}
	void delete_all_children() { //子ノードをすべて削除
		if ((n_type != normal && n_type != document) || n_first == nullptr) return;
		xml_node<T>* node = n_first;
		while (1) {
			if (node->next_node() == nullptr) {
				delete node;
				break;
			} else {
				node = node->next_node();
				delete node->back_node();
			}
		}
		n_first = nullptr;
		n_end = nullptr;
	}
	xml_node<T>* delete_next_node() {
		if (n_type == document || n_next == nullptr) return nullptr;
		xml_node<T>* out = n_next->next_node();
		delete n_next;
		return out;
	}
	xml_node<T>* delete_next_node(const string& n) {
		xml_node<T>* out = nullptr;
		xml_node<T>* node = n_next;
		//検索
		while (node != nullptr) {
			if ((node->type() == normal || node->type() == pi) && node->name() == n) { out = node->next_node(); delete node; break; }
			node = node->next_node();
		}
		return out;
	}
	xml_node<T>* delete_back_node() {
		if (n_type == document || n_back == nullptr) return nullptr;
		xml_node<T>* out = n_back->back_node();
		delete n_back;
		return out;
	}
	xml_node<T>* delete_back_node(const string& n) {
		xml_node<T>* out = nullptr;
		xml_node<T>* node = n_back;
		//検索
		while (node != nullptr) {
			if ((node->type() == normal || node->type() == pi) && node->name() == n) { out = node->back_node(); delete node; break; }
			node = node->back_node();
		}
		return out;
	}
	//属性削除
	void delete_attribute(const string& n) { //指定した属性を削除
		if (a_list.empty()) return;
		xml_attribute<T>* attr = find_attribute(n);
		if (attr != nullptr) {
			delete attr;
			a_list.erase(n);
		}
	}
	void delete_all_attribute() { //属性をすべて削除
		if (a_list.empty()) return;
		for (auto it : a_list) {
			it.second->n_parent = nullptr;
			delete it.second;
		}
		a_list.clear();
	}
};

template <typename T = char>
class xml_document {
	xml_node<T> doc;

	inline int espape_decode(const string& xml, size_t& pos, string& value) {
		if      (xml.compare(pos+1, 3, "lt;")   == 0) { value += '<';  pos += 4; } //&lt; <
		else if (xml.compare(pos+1, 3, "gt;")   == 0) { value += '>';  pos += 4; } //&gt; >
		else if (xml.compare(pos+1, 4, "amp;")  == 0) { value += '&';  pos += 5; } //&amp; &
		else if (xml.compare(pos+1, 5, "quot;") == 0) { value += '\"'; pos += 6; } //&quot; "
		else if (xml.compare(pos+1, 1, "#")     == 0) { //Unicode
			unsigned int utf32;
			int numflag = 0;
			string numstr = "";
			if (xml.compare(pos+2, 1, "x") == 0) {
				//&#xN~N; (16進数表記) N=0~f
				pos += 3;
				while (numflag == 0) {
					switch (xml[pos]) {
						case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
						case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
							numstr += xml[pos];
							break;
						case ';': numflag = 1; break;
						default: return -1;
					}
					++pos;
				}
				utf32 = stoul(numstr, nullptr, 16);
			} else {
				//&#N~N; (10進数表記) N=0~9
				pos += 2;
				while (numflag == 0) {
					switch (xml[pos]) {
						case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
							numstr += xml[pos];
							break;
						case ';': numflag = 1; break;
						default: return -1;
					}
					++pos;
				}
				utf32 = stoul(numstr, nullptr, 10);
			}
			if (utf32 <= 0x007F) { //1byte
				value += (char)utf32;
			} else if (utf32 >= 0x0080 && utf32 <= 0x07FF) { //2byte
				value += (char)(0xC0 | ((utf32 & 0x7C0) >> 6));
				value += (char)(0x80 |  (utf32 & 0x3F));
			} else if (utf32 >= 0x0800 && utf32 <= 0xFFFF) { //3byte
				value += (char)(0xE0 | ((utf32 & 0xF000) >> 12));
				value += (char)(0x80 | ((utf32 & 0xFC0) >> 6));
				value += (char)(0x80 |  (utf32 & 0x3F));
			} else if (utf32 >= 0x10000 && utf32 <= 0x1FFFFF) { //4byte
				value += (char)(0xF0 | ((utf32 & 0x1C0000) >> 18));
				value += (char)(0x80 | ((utf32 & 0x3F000) >> 12));
				value += (char)(0x80 | ((utf32 & 0xFC0) >> 6));
				value += (char)(0x80 |  (utf32 & 0x3F));
			}
		} else return -3;
		return 0;
	}
	//読み込み処理
	int parse_type(const string& xml, size_t& pos, xml_node<T>* parent) {
		int err = 0;
		if (xml[pos] == '/') { ++pos; err = 1; } //終了タグ
		else if (xml[pos] == '!') {
			if (pos+2 < xml.size() && xml[pos+1] == '-' && xml[pos+2] == '-')  { pos += 3; err = parse_comment(xml, pos, parent); }
			else if (xml.compare(pos+1, 8, "DOCTYPE ") == 0) { pos += 9; err = parse_doctype(xml, pos, parent); }
			else err = -1; //該当タグなし
		} else if (xml[pos] == '?') { ++pos; err = parse_pi(xml, pos, parent); }
		else { err = parse_node(xml, pos, parent); }
		return err;
	}
	int parse_name(const string& xml, size_t& pos, string& out) {
		out = "";
		//タグ名
		while(pos < xml.size()) {
			switch (xml[pos]) {
				case ' ': case '\t': case '\n':
					++pos;
					if (out.empty()) return -3; //名前が存在しない
					return 0; //属性処理・内容処理
				case '=':
					++pos;
					if (out.empty()) return -3; //名前が存在しない
					return 3;
				case '>':
					++pos;
					return 1; //内容処理
				case '/':
					if (xml[pos+1] != '>') return -1;
					pos += 2;
					if (out.empty()) return -3; //名前が存在しない
					return 2; // "/>" ノード処理終了
				case '?':
					if (xml[pos+1] != '>') return -1;
					pos += 2;
					return 4; //pi終了
				case ';': case ':': case ',': case '&':
					return -2; //禁止文字
				default:
					out += xml[pos];
					break;
			}
			++pos;
		}
		return -1;
	}
	int parse_attribute(const string& xml, size_t& pos, xml_node<T>* node) {
		string attr_name;
		string attr_value;
		int err;
		//属性
		int attr_mode = 0; //0 = name, 1=equal, 2=value_1, 3=value_2
		while (pos < xml.size()) {
			//終了判定
			if (xml[pos] == '>') {
				if (attr_name.empty() == false) return -1; //作成途中の属性がある
				++pos;
				return 0;
			} else if (xml[pos] == '/') {
				if (xml[pos+1] != '>') return -1;
				if (attr_name.empty() == false) return -1; //作成途中の属性がある
				pos += 2;
				return 2; // "/>" ノード処理終了
			}

			//処理
			if (attr_mode == 0) {
				//空白を省略
				switch (xml[pos]) {
					case ' ': case '\t': case '\n': ++pos; break;
					default: attr_mode = 1; break;
				}
			} else if (attr_mode == 1) {
				//name
				err = parse_name(xml, pos, attr_name);
				if (err < 0) return -1; //属性名の取得に失敗
				else if (err == 3) attr_mode = 3; // '=' が存在
				else if (err == 0) attr_mode = 2;
				else return err;
			} else if (attr_mode == 2) {
				// = が見つかるまで進める
				switch (xml[pos]) {
					case '=': attr_mode = 3; break; //次のモードに移行
					case ' ': case '\t': case '\n': break; //空白文字を省略
					default: return -3; //禁止文字
				}
				++pos;
			} else if (attr_mode == 3) {
				// " が見つかるまで進める
				switch (xml[pos]) {
					case '\"': attr_mode = 4; break; //次のモードに移行
					case ' ': case '\t': case '\n': break; //空白文字を省略
					default: return -3; //禁止文字
				}
				++pos;
			} else if (attr_mode == 4) {
				//value
				switch (xml[pos]) {
					case '\"':
						node->add_attribute(attr_name, attr_value); //属性登録
						attr_name = ""; attr_value = ""; //リセット
						attr_mode = 0; //最初に戻る
						break;
					case '&':
						//特殊文字
						err = espape_decode(xml, pos, attr_value);
						if (err < 0) return err;
						break;
					default:
						attr_value += xml[pos];
						break;
				}
				++pos;
			}
		}
		return -1;
	}
	int parse_node(const string& xml, size_t& pos, xml_node<T>* parent) { //<name>value</name>
		/*----------開始タグ----------*/
		string node_name;
		int err = parse_name(xml, pos, node_name);
		if (err == 2) { parent->add_end_child(node_name, ""); return 0; } // "/>"
		else if (err < 0)  return err;
		else if (err == 3) return -3;
		else if (err == 4) return -4; //pi
		//ノード作成
		xml_node<T>* node = parent->add_end_child(node_name, "");
		if (err == 0) {
			err = parse_attribute(xml, pos, node);
			if (err < 0) return err;
			else if (err == 2) return 0;
		}
		/*----------開始タグ----------*/
		//内容
		string node_value;
		size_t end_tag;
		// "<"が見つかるまで繰り返し
		int value_flag = 0;
		while (1) {
			if (pos >= xml.size()) return -1;
			if (xml[pos] == '<') {
				if (xml.compare(pos+1, 8, "![CDATA[") == 0) {
					//<![CDATA[]]>
					pos += 9;
					end_tag = xml.find("]]>", pos); // "]]>"を検索
					if (end_tag == string::npos) return -1; //タグの終端が存在しない
					node_value += xml.substr(pos, end_tag-pos); //内容を切り取り
					pos = end_tag + 3;
					value_flag = 1;
				} else { ++pos; break; }
			} else if (xml[pos] == '&') {
				//特殊文字
				err = espape_decode(xml, pos, node_value);
				if (err < 0) return err;
				value_flag = 1;
			} else if (xml[pos] == ' ' || xml[pos] == '\n' || xml[pos] == '\t') { node_value += xml[pos]; ++pos; }
			else { value_flag = 1; node_value += xml[pos]; ++pos; }
		}
		if (value_flag == 1) node->value(node_value); //内容書き込み
		//子ノード
		while (1) {
			err = parse_type(xml, pos, node);
			if (err < 0) return err;
			if (err == 1) break;
			end_tag = xml.find('<', pos);
			if (end_tag == string::npos) return -1; //error
			pos = end_tag + 1;
		}
		/*----------終了タグ----------*/
		string end_name;
		err = parse_name(xml, pos, end_name);
		if (err < 0) return err;
		else if (err == 4) return 4; //pi
		if (end_name != node_name) return -5; //ノード名が違う
		if (err == 0) {
			// ">" まで進む
			while (1) {
				if (pos >= xml.size()) return -1;
				if (xml[pos] == '>') { ++pos; break; }
				if (xml[pos] != ' ' && xml[pos] != '\n' && xml[pos] != '\t') return -6;
				++pos;
			}
		}
		/*----------終了タグ----------*/
		return 0;
	}
	int parse_comment(const string& xml, size_t& pos, xml_node<T>* parent) { //<!--value-->
		size_t end_tag = xml.find("-->", pos); // "-->"を検索
		if (end_tag == string::npos) return -1; //タグの終端が存在しない
		string val = xml.substr(pos, end_tag-pos); //内容を切り取り
		parent->add_end_child("", val, comment); //コメントタグを追加
		pos = end_tag + 3;
		return 0;
	}
	int parse_doctype(const string& xml, size_t& pos, xml_node<T>* parent) { //<!DOCTYPE>
		string val;
		size_t end_tag;
		while(pos < xml.size()) {
			switch (xml[pos]) {
				case '>':
					//終了
					parent->add_end_child("", val, doctype); //DOCTYPEタグを追加
					++pos;
					return 0;
				case '[':
					++pos;
					end_tag = xml.find("]", pos); // ']' を検索
					if (end_tag == string::npos) return -1; // ']' が存在しない
					val += "[" + xml.substr(pos, end_tag-pos) + "]";
					pos = end_tag + 1;
					break;
				case '<': case ']': case ';': case ':': case ',': case '&':
					//禁止文字
					return -2;
				default:
					val += xml[pos];
					++pos;
					break;
			}
		}
		return -1;
	}
	int parse_pi(const string& xml, size_t& pos, xml_node<T>* parent) { //<?value?>
		size_t end_tag = xml.find("?>", pos); // "?>"を検索
		if (end_tag == string::npos) return -1; //タグの終端が存在しない
		string val = xml.substr(pos, end_tag-pos); //内容を切り取り
		parent->add_end_child("", val, pi); //PIタグを追加
		pos = end_tag + 2;
		return 0;
	}

	//書き出し処理
	string print_value_out(const string& val) { //特殊文字の変換
		string out;
		size_t pos = 0;
		while (pos < val.size()) {
			switch (val[pos]) {
				case '<':  out += "&lt;";   break;
				case '>':  out += "&gt;";   break;
				case '&':  out += "&amp;";  break;
				case '\"': out += "&quot;"; break;
				case '\'': out += "&apos;"; break;
				default:   out += val[pos]; break;
			}
			++pos;
		}
		return out;
	}
	void print_attribute(string& xml, xml_node<T>* node) {
		xml_attribute<T>* attr = nullptr;
		for (auto it : node->a_list) {
			attr = it.second;
			if (attr->name().empty() == false) xml += " " + attr->name() + "=\"" + print_value_out(attr->value()) + "\"";
		}
	}
	void print_child(string& xml, xml_node<T>* node, int indent, const string& indenttext, const string& new_line) {
		xml_node<T>* child = node->first_child();
		size_t next_indent;
		if (indent >= 0) {
			if (node->n_type == document) next_indent = indent;
			else next_indent = indent + 1;
		} else next_indent = -1;

		while (child != nullptr) {
			switch (child->type()) {
				case doctype: print_doctype(xml, child, next_indent, indenttext, new_line); break;
				case comment: print_comment(xml, child, next_indent, indenttext, new_line); break;
				case normal:  print_normal(xml, child, next_indent, indenttext, new_line);  break;
				case pi:      print_pi(xml, child, next_indent, indenttext, new_line);      break;
				default: break;
			}
			child = child->next_node();
		}
	}
	void print_doctype(string& xml, xml_node<T>* node, int indent, const string& indenttext, const string& new_line) {
		for (int i = 0; i < indent; i++) xml += indenttext; //インデント
		xml += "<!DOCTYPE " + node->value() + ">" + new_line;
	}
	void print_normal(string& xml, xml_node<T>* node, int indent, const string& indenttext, const string& new_line) {
		for (int i = 0; i < indent; i++) xml += indenttext; //インデント
		xml += "<" + node->name(); //開始タグ
		print_attribute(xml, node);
		if (node->value().empty() == false || node->first_child() != nullptr) {
			xml += ">" + print_value_out(node->value());
			if (node->n_first != nullptr) {
				xml += new_line;
				print_child(xml, node, indent, indenttext, new_line);
				for (int i = 0; i < indent; i++) xml += indenttext; //インデント
			}
			xml += "</" + node->name() + ">" + new_line; //終了タグ
		} else xml += "/>" + new_line;
	}
	void print_comment(string& xml, xml_node<T>* node, int indent, const string& indenttext, const string& new_line) {
		for (int i = 0; i < indent; i++) xml += indenttext; //インデント
		xml += "<!--" + node->value() + "-->" + new_line;
	}
	void print_pi(string& xml, xml_node<T>* node, int indent, const string& indenttext, const string& new_line) {
		for (int i = 0; i < indent; i++) xml += indenttext; //インデント
		xml += "<?" + node->value() + "?>" + new_line;
	}
public:
	xml_document() { doc.type(document); }
	xml_document(const string& text) { doc.type(document); parse(text); }
	~xml_document() { clear(); }

	void clear() { doc.clear(); } //データの解放

	xml_node<T>* get() { return &doc; }

	int parse(const string& text) { //XMLの読み込み
		clear(); //以前のデータを開放

		size_t pos = text.find('<'); //最初のタグを検索
		while (pos != string::npos) {
			++pos;
			//処理
			int err = parse_type(text, pos, &doc);
			if (err < 0) return err; //失敗
			pos = text.find('<', pos);
		}
		return 0;
	}
	string print(bool indent = true, const string& indenttext = "\t") { //XMLの書き出し
		string out;
		print_child(out, &doc, indent ? 0 : -1, indenttext, indent ? "\n" : "");
		return out;
	}
};

#endif //_MYXML_HPP
