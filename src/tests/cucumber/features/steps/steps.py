# Gherkins test steps definitions

import os
from behave import *
from simulator_utils import *
from assertions import *
from context_utils import *

@given('the study path is "{string}"')
def study_path_is(context, string):
    context.study_path = os.path.join("..", "resources", "Antares_Simulator_Tests_NR" , string.replace("/", os.sep))

@when('I run antares simulator')
def run_antares(context):
    context.use_ortools = True
    context.ortools_solver = "sirius"
    context.named_mps_problems = False
    context.parallel = False
    run_simulation(context)

@then('the simulation succeeds')
def simu_success(context):
    assert context.return_code == 0

@then('the simulation fails')
def simu_success(context):
    assert context.return_code != 0

@then('the expected value of the annual system cost is {value}')
def check_annual_cost_expected(context, value):
    assert_double_close(float(value), get_annual_system_cost(context)["EXP"], 0.001)

@then('the minimum annual system cost is {value}')
def check_annual_cost_min(context, value):
    assert_double_close(float(value), get_annual_system_cost(context)["MIN"], 0.001)

@then('the maximum annual system cost is {value}')
def check_annual_cost_max(context, value):
    assert_double_close(float(value), get_annual_system_cost(context)["MAX"], 0.001)

@then('the annual system cost is')
def check_annual_cost(context):
    for row in context.table:
        assert_double_close(float(row["EXP"]), get_annual_system_cost(context)["EXP"], 0.001)
        assert_double_close(float(row["STD"]), get_annual_system_cost(context)["STD"], 0.001)
        assert_double_close(float(row["MIN"]), get_annual_system_cost(context)["MIN"], 0.001)
        assert_double_close(float(row["MAX"]), get_annual_system_cost(context)["MAX"], 0.001)

@then('the simulation takes less than {seconds} seconds')
def check_simu_time(context, seconds):
    actual_simu_time = parse_simu_time(context.output_path)
    assert actual_simu_time <= float(seconds)

@then('in area "{area}", during year {year}, loss of load lasts {lold_hours} hours')
def check_lold_duration(context, area, year, lold_hours):
    assert int(lold_hours) == get_hourly_values(context, area.lower(), int(year))["LOLD"].sum()

@then('in area "{area}", unsupplied energy on "{date}" of year {year} is of {lold_value_mw} MW')
def check_lold_value(context, area, date, year, lold_value_mw):
    actual_unsp_energ = get_hourly_values_for_specific_hour(context, area.lower(), int(year), date)["UNSP. ENRG"].sum()
    assert_double_close(float(lold_value_mw), actual_unsp_energ, 0.001)

def after_feature(context, feature):
    # post-processing a test: clean up output files to avoid taking up all the disk space
    if (context.output_path != None):
        pathlib.Path.rmdir(context.output_path)
