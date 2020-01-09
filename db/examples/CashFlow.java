
public void doIt(boolean opt, int id) {
	String sql = new String();
	ResultSet rs = null;
    sql = buildQuery(opt, id);
	rs = executeQuery(sql);
    while (rs.next()){
    	doSomething(rs);
    }
}
protected String buildQuery(boolean opt, int id){
	StringBuilder sql = "";
	if (opt){
	  if (id < 0)
        sql.append("SELECT A,B,sqlfunc(" + id + ") FROM table1")
      else
      	sql.append("SELECT A,B,C FROM table1")
    }
	else
	  if (id < 0)
        sql.append("SELECT A,B,sqlfunc("+ id + ") FROM table1 WHERE B='y'")
      else
      	sql.append("SELECT A,B,C FROM table1 WHERE B='y'")
	return sql.toString();
}
