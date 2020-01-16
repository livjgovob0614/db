

public int check(int inputYear) {
  int count = calStat(inputYear);
  if (count) {
    assert(count < 10 && "error");
    return count;
  }
  return -1;
}

public int calStat(int inputYear) {
  conn = new SQLConnection();
  sql = buildQuery("SELECT t.income, t.balance, t.year from t where t.year = " + inputYear);

  results = executeQuery(sql);
  while(results.read()) {
    int income = results.getInt(1);
    int balance = results.getInt(2);
    int year = reults.getInt(3);
    int diff = (income - 1.5 * balance) * year;
    if (diff > 10000)
      count++;
  }
  return count;
}

