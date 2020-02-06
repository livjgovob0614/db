
public CurrentYear computeCurrentYear(int[] totals, int value, String startOfYear) {
  
// 사실 String은 영 아닌데...... jooq 어떤 변수로 이거 받을 수 있는지 낼  확인할게요
// 미팅 때는 그냥 var로 보여드리고 설명하면 될 것 같아요
  String total = sum(MONTYLY_MILAGES.field(MONTHLY_MILAGE_VALUE)).as("total");
  String idxA = MONTHLY_MILAGES.filed(MILAGES.RECORDED_ON).as("idx");

  this.database
          .select(idxA,
                  total)
          .from(MONTHLY_MILAGES)
          .where(MONTHLY_MILAGES.field(RECORDED_ON).isNotNull())
          .and(MONTHLY_MILAGES.filed(RECORDED_ON)
                  .greaterOrEqual(startOfYear);
          .orderBy(MONTHLY_MILAGES.field(BIKES.NAME).asc())
          .forEach(record -> {
            int index = record.get(idxA).intValue();
            if (totals[index] == -1) {
              totals[index] = record.get(total).intValue();
            }
          });

  for (int i=0; i<totals.length; ++i) {
    int currentValue = totals[i];
    if (currentValue == -1) {
      totals[i] = 0;
    } else {
      if (currentValue > maxValue) {
        maxValue = currentValue;
        maxIndex = i;
      }
      if (currentValue < minValue) {
        minValue = currentValue;
        minIndex = i;
      }
    }
  }

  return (new CurrentYear(totals, minIndex+1, minValue, maxIndex+1, maxValue));
}
