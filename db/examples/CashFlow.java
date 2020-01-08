import java.sql.*;
//import java.sql.PreparedStatement;
//import java.sql.ResultSet;
//import java.sql.SQLException;
//import java.sql.Connection;
import java.util.ArrayList;
import java.util.List;

public class CashFlow {

  protected String getSqlSelect(int option, int clientId) {
    final StringBuffer sql = new StringBuffer();

    switch (option) {
      case 1:
      {
        sql.append("SELECT C_BackAccount_ID, bp.C_BP_Group_ID,"
        + getSqlCashFlowSource(clientId)
        + ","
        + " sqlFunc(" + clientId
        + ") FROM Table1"
        + " WHERE option="
        + option);
        break;
      }
      case 2:
      {
        sql.append("SELECT A_ID,"
        + getSqlCashFlowSource(clientId)
        + ","
        + " sqlFunc(" + clientId
        + ") FROM Table1");
        break;
      }
      default:
      {
        return null;
      }
    }

    return sql.toString();
  }

  protected String getSqlCashFlowSource(int clientId) {
    String src = null;
    if (clientId > 1000)
      src = new String("G_ID");
    else
      src = new String("H_ID");
    return src;
  }

  protected PreparedStatement prepareStatement(String sql) {
    Connection conn = null;
    PreparedStatement stat = null;

    try {
      conn = getConnection();
      stat = conn.prepareStatement(sql);
      return stat;
    } catch (Exception e) {
      System.out.println("exception");
    }
    return null;
	}

	protected Connection getConnection() {
		Connection conn = null;
    String url = "asdf";
    String id = "test";
    String pw = "1234";
    try {
      Class.forName("oracle....");
      conn = DriverManager.getConnection(url, id, pw);
      return conn;
    } catch (Exception e) {
      System.out.println("exception");
    }
    return null;
	}


  public void doIt(int option, int clientId) throws Exception {
    StringBuffer sql = new StringBuffer();
    sql.append(getSqlSelect(option, clientId));

    PreparedStatement pstmt = null;
    ResultSet rs = null;
    try {
      pstmt = prepareStatement(sql.toString());
      rs = pstmt.executeQuery();
      while (rs.next()) {
        System.out.println(rs); 
      }
    } catch (SQLException e) {
      System.out.println("exception");
    } finally {
      pstmt.close();
      rs = null;
      pstmt = null;
    }
  }
}
