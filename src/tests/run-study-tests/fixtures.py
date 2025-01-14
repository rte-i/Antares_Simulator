import pytest

from actions_on_study.study_run import *
from actions_on_study.results_remover import *
from check_on_results.check_general import check_list

class check_handler:
    def __init__(self, simulation, results_remover):
        self.simulation = simulation
        self.results_remover = results_remover
        self.checks = check_list()

    def get_simulation(self):
        return self.simulation

    def run(self, checks):
        # Runs the simulation on the current study
        self.simulation.run()
        # Stores check list for teardown
        self.checks = checks
        # Runs the checks to be performed on the current study.
        checks.run()

    def teardown(self):
        study_modifiers = self.checks.study_modifiers()
        for modifier in study_modifiers:
            modifier.back_to_initial_state()

        self.results_remover.run()

# ================
# Fixtures
# ================
@pytest.fixture()
def study_path(request):
    return request.param

@pytest.fixture
def resultsRemover(study_path):
    return results_remover(study_path)

@pytest.fixture
def simulation(study_path, antares_simu_path, solver_name, named_mps_problems, parallel):
    return study_run(study_path, antares_simu_path, solver_name, named_mps_problems, parallel)

@pytest.fixture(autouse=True)
def check_runner(simulation, resultsRemover):
    # Actions done before the current test
    my_check_handler = check_handler(simulation, resultsRemover)

    # A check handler is supplied to the current test now
    yield my_check_handler

    # Teardown : actions done after the current test
    my_check_handler.teardown()
