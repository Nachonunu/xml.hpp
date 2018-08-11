#include "xml.hpp"

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
	//読み込み
	string xml = "\
		<xml>\
			<!-- <>;:&\"\' -->\
			<a attr=\"attr value\">&lt;node value&#x3042;</a>\
			<b attr = \"attr1\"/>\
			<c>A<![CDATA[日本語]]>B</c>\
			<?PI?>\
		</xml>";
	xml_document<> doc(xml);

	//取得
	xml_node<>* obj = doc.get();
	xml_node<>* n = obj->first_child()->first_child("a");
	xml_attribute<>* a = n->find_attribute("attr");
	cout << n->name() << " " << n->value() << endl;
	cout << a->name() << " " << a->value() << endl;

	//編集
	//n->name("ChangeNodeName"); n->value("ChangeNodeValue");
	a->name("ChangeAttrName"); a->value("ChangeAttrValue");

	//ノードの追加
	xml_node<>* an = n->add_next_node("AddNode", "日本語");
	//属性の追加
	an->add_attribute("Add1", "1");
	an->add_attribute("Add2", "2");
	an->add_attribute("Add3", "3");

	//削除
	an->delete_attribute("Add2");
	delete an->next_node();

	//書き出し
	ofstream ofs("out.xml");
	ofs << doc.print();
	return 0;
}
