"""Command line for the Studio's tooling: `python3 -m studio <command>`."""

import argparse
import sys

from studio import axes, batch, render, report, states


def _render(args):
    path = render.render(args.variant, args.state, args.output)
    print(path)


def _batch(args):
    # The report is built here rather than inside render_batch so that batch
    # does not have to import report, which reads batch to find the renders.
    result = batch.render_batch(
        args.session, args.variants,
        on_progress=lambda variant: print('rendered', variant, flush=True))
    print('contact sheet:', result['contact_sheet'])
    print('report:', report.build(args.session, variants=args.variants))


def _report(args):
    print(report.build(args.session))


def _pick(args):
    print(report.record_pick(args.session, args.variant, args.why))


def _variants(args):
    for variant in render.available_variants():
        positions = axes.positions_of(variant, render.VARIANTS_DIR)
        print('{:12} {}'.format(
            variant, ' · '.join(positions[axis] for axis in axes.AXIS_ORDER)))


def main(argv=None):
    parser = argparse.ArgumentParser(prog='studio', description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)

    render_command = commands.add_parser(
        'render', help='Render one Variant at one state.')
    render_command.add_argument('variant', help='Variant name, e.g. plain')
    render_command.add_argument('state', nargs='?', default='canonical',
                                choices=sorted(states.BY_NAME))
    render_command.add_argument('-o', '--output', default=None)
    render_command.set_defaults(handler=_render)

    batch_command = commands.add_parser(
        'batch', help='Render a Batch of six Variants and tile a Contact sheet.')
    batch_command.add_argument('session', help='Session name, e.g. 2026-07-24-mono')
    batch_command.add_argument('variants', nargs='+',
                               help='The six Variants in the Batch')
    batch_command.set_defaults(handler=_batch)

    report_command = commands.add_parser(
        'report', help='Rebuild the report for a Session already rendered.')
    report_command.add_argument('session')
    report_command.set_defaults(handler=_report)

    pick_command = commands.add_parser(
        'pick', help='Record the winning Variant and close the Session.')
    pick_command.add_argument('session')
    pick_command.add_argument('variant')
    pick_command.add_argument('--why', default=None)
    pick_command.set_defaults(handler=_pick)

    variants_command = commands.add_parser(
        'variants', help='List the Variants in the Studio.')
    variants_command.set_defaults(handler=_variants)

    args = parser.parse_args(argv)
    if args.command == 'render' and args.output is None:
        args.output = 'build/renders/{}_{}.png'.format(args.variant, args.state)

    try:
        args.handler(args)
    except (render.RenderError, batch.BatchError, KeyError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
