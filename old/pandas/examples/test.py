import sys

import pandas as pd
import numpy as np


def test_example(target_csv):
    result = False

    df = pd.read_csv(target_csv)
    df2 = df[(df['nametype'] == 'Valid') & (df['id'] < 10.0)]

    if (df2.loc[0,"name"] == 'Aachen'):
      result = True

    print(result)




test_example("test.csv")

