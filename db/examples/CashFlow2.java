
SQLResultSet doIt(int id, SQLResultSet rs) {
	resultSet = null;
  sql = buildQuery(id);

	resultSet = executeQuery(sql);
  while (resultSet.next()){
  	doSomething(result);
  }
}

String buildQuery(int id){
	StringBuilder sql = "";
	if (id > 0)
      sql.append("SELECT A,B,sqlfunc(" + id + ") FROM table1")
    else
    	sql.append("SELECT A,B,C FROM table1")
  }
	return sql.toString();
}

int doSomething(result) {
  updateNewTable(

}
