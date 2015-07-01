#pragma once


/*
第一行作为列名:
按指定类型加载指定列{"name:s", "hp:d"},可以用"..."指代其余所有列
如果不指定加载列,就加载所有列
值类型默认为字符串: s: 字符串, d:整数, u:无符号整数,f:浮点数(double)
可以指定某列作为索引,如果不指定,则以行号为索引
示例: tab = LoadTabFile("data.tab", {"id:u", "name:s", "...:d"}, "id");
注意,除了文件名以外,后面的加载列参数和索引列名两个参数都是可选的,顺序也无所谓
*/
int LuaLoadTabFile(lua_State* L);


/*
第一行不做列名,而是普通数据行
不能指定加载哪些列,但是可以为所有列指定一个类型
不能指定索引列
示例: tab = LoadTabData("data.tab", "d");
*/
int LuaLoadTabData(lua_State* L);

