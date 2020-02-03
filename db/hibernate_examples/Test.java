import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;
import javax.persistence.TypedQuery;
import javax.persistence.criteria.*;
import java.util.ArrayList;
import java.util.List;

public Test {
	protected int run(int inputAge) {
//Begin
    EntityManager em = new EntityManager();
    CriteriaBuilder builder = em.getCriteriaBuilder();
    //CriteriaQuery<...> query = builder.createQuery(....class);
    CriteriaQuery<Object> query = builder.createTupleQuery;

    Root<Obj1> obj1 = query.from(Obj1.class);
    Join<Obj1, Obj2> obj2 = obj1.join("obj2", JoinType.INNER);

    query.multiselect(obj1.get("age"), obj2)
         .where(builder.equal(obj2.get("username"), "Kim"));
    TypedQuery<Tuple> query2 = em.createQuery(query);
//End

//Query = "SELECT t1.age, t2.* FROM t1, t2 WHERE t2.username='Kim'";
    List<Tuple> resultList = query2.getResultList();
    for (Tuple tuple : resultList) {
      Int age = tuple.get("age", Int); //?
      Obj2 ob2 = tuple.get("obj2", Obj2.class);

      if (inputAge < age) 
	    return age - inputAge;
	}

    return 0;
  }

  public static void main(String[] args) {
		int age = 10;
		int result = run(age);

		if (result)
			System.out.println("**"); 
  }
}
