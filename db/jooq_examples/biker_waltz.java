
public CurrentYear computeCurrentYear(int[] totals, String startOfYear) {
  
// 사실 String은영 아닌 것 같고  jooq에서 뭘로 이거 받을 수 있는지 낼 확인할게요
// 미팅 때는 그냥 'var'로 보여드리고 설명해도 될 것 같아요
  String total = sum(MONTYLY_MILAGES.field(MONTHLY_MILAGE_VALUE)).as("total");
  String idxA = MONTHLY_MILAGES.filed(RECORDED_ON).as("idx");

  this.database
          .select(idxA,
                  total)
          .from(MONTHLY_MILAGES)
          .where(MONTHLY_MILAGES.field(MONTHLY_MILAGE_VALUE).isNotNull())
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
