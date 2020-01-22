import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.ManyToOne;
import javax.persistence.Table;

@Entity
@Table(name="obj1")
public class Obj1 {

  @Id
  @GeneratedValue
  private String username;
  private int phone;


  @OneToOne
  @JoinColumn(name = "JOIN_COLUMN")
  private Obj2 obj2;
}

@Entity
@Table(name="obj2")
public class Obj2 {
  @Id
  @GeneratedValue
  private String username;
  private int age;
  private int height;
  private int weight;
  private char bloodtype;
}
