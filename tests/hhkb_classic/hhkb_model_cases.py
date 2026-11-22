"""Shared expectations for the Python and standalone browser target checks."""
MODELS = ('AHHX01', 'AHHS01', 'AHKS01', 'AHUX01', 'AHJX01')


def model_cases():
    cases = {}
    for color in 'BW':
        for blank in ('', 'N'):
            cases[f'PD-KB401{color}{blank}'] = 'AHHX01'
    for family, model in (('401', 'AHHS01'), ('421', 'AHKS01')):
        for color in 'BWY':
            for blank in ('', 'N'):
                for revision in ('', '1', '2', '12'):
                    cases[f'PD-KB{family}{color}{blank}SC{revision}'] = model
    for family, model in (('800', 'AHUX01'), ('820', 'AHJX01')):
        for color in 'BWY':
            for blank in ('', 'N'):
                for suffix in ('', 'S', 'SC', 'SC1', 'SC2', 'SC12'):
                    cases[f'PD-KB{family}{color}{blank}{suffix}'] = model
    for number in ('', 'unknown', 'PD-ID100B', 'PD-KB400WS', 'PD-KB420W',
                   'PD-KB401Y', 'PD-KB421W', 'PD-KB401WS', 'PD-KB401WSCX',
                   'PD-KB800', 'PD-KB800ZSC', 'PD-KB800YSC1X', 'PD-KB800YSC-1',
                   'PD-KB800YSC1 extra', 'PD-KB800YSS', 'PD-KB820WSC1X',
                   'PD-KB800W/PD-KB820W', 'PD-KB800WSC1\nPD-KB820WSC1'):
        cases[number] = None
    return cases
