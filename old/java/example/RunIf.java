package smalldemos.ifx;

import jbse.apps.run.RunParameters;
import jbse.apps.run.Run;
import static jbse.apps.run.RunParameters.DecisionProcedureType.Z3;
import static jbse.apps.run.RunParameters.StateFormatMode.TEXT;
import static jbse.apps.run.RunParameters.StepShowMode.LEAVES;

public class RunIf {
    public static void main(String[] args)	{
        final RunParameters p = new RunParameters();
        set(p);
        final Run r = new Run(p);
        r.run();
    }
	
    private static void set(RunParameters p) {
      p.addUserClasspath("./", "/home/jg/java/src/main/java/jbse/meta");
      p.setMethodSignature("smalldemos/ifx/IfExample", "(I)V", "m");
      p.setDecisionProcedureType(Z3);
      p.setExternalDecisionProcedurePath("/opt/local/bin/z3");
      p.setOutputFileName("./out/runIf_z3.txt");
      p.setStateFormatMode(TEXT);
      p.setStepShowMode(LEAVES);
    }
}
