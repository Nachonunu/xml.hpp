# xml.hpp

xmlの読み込み、編集、書き出しを行うライブラリーです。

## 使い方

c++11に対応したコンパイラが必要です。

xml.hppをインクルードするだけで利用することができます。

最初にxml_document<>を初期化してください。

### 読み込み

xml_document<>の初期化時にXMLテキストを第一引数に指定してください。

```cpp
std::string str = "<xml><example attr=\"a\">b</example></xml>";

xml_document<> x(str);
```

xml_document<>のparse関数でも同じことができます。

```cpp
std::string str = "<xml><example attr=\"a\">b</example></xml>";

xml_document<> x;
x.parse(str);
```

### 書き出し

xml_document<>のprint関数を利用してください。

第一引数にインデントの有無(デフォルト true)、第二引数にインデント文字(デフォルト \t)を指定します。

```cpp
xml_document<> x;

//インデントあり、インデント文字を半角スペース4つに設定
std::string out = x.print(true, "    ");
```

### 取得・編集

xml_document<>のget関数を利用します。

```cpp
xml_document<> x;
xml_node<>* node = x.get();

//ノードを追加
xml_node<>* add_node1 = node->add_first_child("AddNode1", "Value");
xml_node<>* add_node2 = node->add_first_child("AddNode2", "");
xml_node<>* add_node2_1 = add_node2->add_first_child("AddNode2_1", "日本語");
xml_node<>* add_node2_2 = add_node2->add_first_child("AddNode2_2", "English");
xml_node<>* add_node2_3 = add_node2->add_first_child("AddNode2_3", "");
xml_node<>* add_node2_4 = add_node2->add_first_child("AddNode2_4", "");
xml_node<>* add_node3 = node->add_first_child("AddNode3", "");

//属性を追加
xml_attribute<>* add_attr1 = add_node1->add_attribute("Attr1", "Value");

//ノードを削除
delete add_node2_2;
add_node2->delete_first_child("AddNode2_3");

//ノードを検索
xml_node<>* search_node = node->first_child();
search_node = search_node->next_node("AddNode3");

//ノードの内容を取得
std::string node_name = add_node1->name();
std::string node_value = add_node1->value();

//ノードの内容を編集
add_node1->name("ChangeName");
add_node1->value("ChangeValue");
```

## ライセンス

[CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/deed)

