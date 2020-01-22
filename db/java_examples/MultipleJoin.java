import javax.persistence.EntityManager;
import javax.persistence.PersistenceContext;
import javax.persistence.TypedQuery;
import javax.persistence.criteria.*;
import java.util.ArrayList;
import java.util.List;

public Test {
	protected int run(int inputAge) {
    EntityManager em = new EntityManager();

    CriteriaBuilder builder = em.getCriteriaBuilder();
    //CriteriaQuery<...> query = builder.createQuery(....class);
    CriteriaQuery<Object> query = builder.createTupleQuery;

    Root<Obj1> obj1 = query.from(Obj1.class);
    Join<Obj1, Obj2> obj2 = obj1.join("obj2", JoinType.INNER);

    Predicate pred = builder.equal(obj2.get("username"), "Kidm");

    query.multiselect(obj1.get("age").alias("age"), obj2.alias("obj2"))
         .where(pred);
    TypedQuery<Tuple> query2 = em.createQuery(query);

    List<Tuple> resultList = query2.getResultList();
    for (Tuple tuple : resultList) {
      Int age = tuple.get("age", Int); //?
      Obj2 ob2 = tuple.get("obj2", Obj2.class);

      if (inputAge < age) {
				return age - inputAge;
			}
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
