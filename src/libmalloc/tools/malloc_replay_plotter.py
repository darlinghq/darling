#!/usr/bin/env python

from __future__ import absolute_import
from __future__ import unicode_literals
from __future__ import division
from __future__ import print_function

import sys
import os
import re
import argparse
import logging
import json
from pprint import pprint
import numpy as np
import matplotlib.pyplot as plt


class ReportConfiguration(object):

    def __init__(self, report_type, call, nano_malloc_cutoff, xfilter, num_bins, merge_calloc, fileV1, fileV2):
        self.report_type = report_type
        self.call = call
        self.nano_malloc_cutoff = nano_malloc_cutoff
        self.xfilter = xfilter
        self.num_bins = num_bins
        self.merge_calloc = merge_calloc
        self.fileV1 = fileV1
        self.fileV2 = fileV2

    def plotter_class(self):
        if self.report_type == "scatter":
            return ScatterPlotter
        if self.report_type == "instructions":
            return InstructionsPlotter
        if self.report_type == "request_sizes":
            return RequestSizePlotter
        if self.report_type == "nano_request_bins":
            return RequestSizePlotter
        if self.report_type == "nano_request_bins_ysize":
            return RequestSizePlotter

    def call_identifier(self):
        return self.call_identifier_for_name(self.call)

    @classmethod
    def call_identifier_for_name(cls, name):
        mapping = {'malloc': 1, 'realloc': 3, 'memalign': 4, 'calloc': 5, 'valloc': 6}
        return mapping[name]

    @classmethod
    def configuration_for_arguments(cls, args):
        return cls(args.report_type, args.call, args.nano_malloc_cutoff, args.xfilter, args.num_bins, args.merge_calloc, args.fileV1, args.fileV2)


class ReportData(object):

    def __init__(self, fileV1, fileV2):
        self.fileV1 = fileV1
        self.fileV2 = fileV2

        self.all_data = []
        self.frag = []
        self.paths = [fileV1, fileV2]

        self.parse_input_files()

    def parse_input_files(self):
        with open(self.fileV1) as f:
            self.all_data.append(json.load(f))
        if self.fileV2:
            with open(self.fileV2) as f:
                self.all_data.append(json.load(f))
        self.calculate_fragmentation()

    def enumerate(self):
        for i, data in enumerate(self.all_data):
            yield i, data, self.frag[i], self.paths[i]

    def fileV1_data(self):
        return self.all_data[0]

    def num_plots(self):
        return 2 if self.fileV1 and self.fileV2 else 1

    def calculate_fragmentation(self):
        for data in self.all_data:
            total_frag = 0
            data = data['data']
            for obj in data:
                for i in obj:
                    if 'variables' in i:
                        if i['metric'] == 'Fragmentation':
                            total_frag += i['value']
            self.frag.append(total_frag)


class Plotter(object):

    def __init__(self, report_configuration):
        self.configuration = report_configuration

    def plot(self, report_data):
        pass

    # Returns a list of sizes requested and the frequency at which this request
    # was made.
    def size_freq_for_data(self, data, call_identifier):
        size_filter = self.configuration.nano_malloc_cutoff
        if not size_filter:
           size_filter = size.maxint

        size_freq = []
        for ext, counts in data['extensions']['libmalloc.instruction_counts'].items():
            if counts['call'] == call_identifier and int(counts['size']) <= size_filter:
                size_freq.append([counts['size'], counts['count']])
        return size_freq

    # Returns a list of lists of ([size, [instruction counts]]). Where size is the
    # requested size and instruction counts are the number of CPU instructions it took (as
    # recorded by libmalloc_replay. If coalesce is set, this instead returns a
    # coalesced list of instruction counts ([instruction counts]), flattened across all
    # request sizes.
    def times_for_data(self, data, call_identifier, coalesce):
        size_filter = self.configuration.nano_malloc_cutoff
        if not size_filter:
            size_filter = size.maxint
        times = []
        for ext, counts in data['extensions']['libmalloc.instruction_counts'].items():
            if counts['call'] == call_identifier and int(counts['size']) <= size_filter:
                if coalesce:
                    times += counts['values']
                else:
                    times.append([counts['size'], counts['values']])
        return times

    def show(self):
        plt.show()

    def write_to_path(self, path):
        plt.savefig(path)


class ScatterPlotter(Plotter):

    def plot(self, report_data):
        plt.figure(figsize=(20,10))
        labels = ["V1", "V2"]
        colours = ['r', 'b']
        for i, data, _, _ in report_data.enumerate():
            logging.debug("Building data")
            sizecounts = self.times_for_data(data, self.configuration.call_identifier(), False)
            sizes = []
            counts = []
            for pair in sizecounts:
                rsize = pair[0]
                for icount in pair[1]:
                    sizes.append(rsize)
                    counts.append(icount)
            colmark = colours[i] + 'x'
            logging.debug("Plotting scatter")
            scatter, = plt.plot(sizes, counts, colmark)
            scatter.set_label(labels[i])
        plt.xlabel("Requested Size (Bytes)")
        plt.ylabel("Instruction Count")
        plt.legend()


class InstructionsPlotter(Plotter):

    def plot(self, report_data):
        num_plots = report_data.num_plots()
        fig = plt.figure(figsize=(20, num_plots * 5))
        subplot_config = 221 if num_plots == 2 else 121

        for i, data, fragmentation, path in report_data.enumerate():
            all_times = self.times_for_data(data, self.configuration.call_identifier(), True)

            # We may want to just filter for a certain range (0, xfilter)
            if self.configuration.xfilter:
                filtered = [t for t in all_times if t < self.configuration.xfilter]
            else:
                filtered = all_times

            logging.debug("Plotting: Histogram")
            # Histogram
            h_ax = plt.subplot(subplot_config)
            subplot_config += 1
            self.hist_data(filtered, False, 1)
            if self.configuration.xfilter > 0:
                h_ax.set_xlim([0, self.configuration.xfilter.xfilter])

            plt.suptitle('{}: {}'.format(path, self.configuration.call))

            logging.debug("Plotting: CDF")
            # CDF
            ax = plt.subplot(subplot_config)
            subplot_config += 1
            self.hist_data(all_times, True, 0)

            # Table
            logging.debug("Producing table")
            per50 = np.percentile(all_times, 50)
            per75 = np.percentile(all_times, 75)
            per95 = np.percentile(all_times, 95)

            tblstr = 'Fragmentation: {}%\n50th: {}\n75th: {}\n95th: {}'.format(fragmentation, per50, per75, per95)
            ax.text(0, 0.1, tblstr, bbox=dict(facecolor='white'), horizontalalignment='right', verticalalignment='top')

    def hist_data(self, data, cumulative, width):
        norm = 1 if cumulative else 0
        plt.hist(data, bins=self.configuration.num_bins, log=False, cumulative=cumulative, linewidth=width, normed=norm)
        plt.xlabel("Instruction Counts")
        if cumulative:
            plt.title("Cumulative")


class RequestSizePlotter(Plotter):

        def sort_split_and_fill_size_freqs(self, size_freq):
            # Sort by the size. Then split into two lists.
            size_freq.sort(key=lambda x: x[0])
            sizes = [i[0] for i in size_freq]
            counts = [i[1] for i in size_freq]

            # Fill out the arrays where we didn't see events. This helps when we
            # bin the data later.
            sparse_sizes = []
            sparse_counts = []
            i = 0
            j = 0
            while i < max(sizes):
                if sizes[j] == (i + 1):
                    sparse_sizes.append(sizes[j])
                    sparse_counts.append(counts[j])
                    j = j + 1
                else:
                    sparse_sizes.append(i+1)
                    sparse_counts.append(0)
                i = i + 1
            return sparse_sizes, sparse_counts

        def merge_size_counts(self, sizes, counts, sizes_c, counts_c):
            # Merge the calloc data with malloc. N.b. The lists can be different lengths; merge into sizes.
            if len(sizes_c) > len(sizes):
                tmpS = sizes
                tmpC = counts
                sizes = sizes_c
                counts = counts_c
                sizes_c = tmpS
                counts_c = tmpC
            for i in range(len(sizes)):
                if i >= len(sizes_c):
                    break
                assert(sizes[i] == sizes_c[i])
                counts[i] = counts[i] + counts_c[i]
            return sizes, counts

        def plot(self, report_data):
            plt.figure(figsize=(50, 10))
            plt_config = 211
            calls = ['malloc']
            if not self.configuration.merge_calloc:
                plt_config = 311
                calls.append('calloc')
            calls.append('realloc')

            for call_name in calls:
                logging.debug('Plotting: %s' % call_name)
                call_identifier = self.configuration.call_identifier_for_name(call_name)

                data = report_data.fileV1_data()
                size_freq = self.size_freq_for_data(data, call_identifier)
                sizes, counts = self.sort_split_and_fill_size_freqs(size_freq)

                # calloc merging
                if self.configuration.merge_calloc and call_name == 'malloc':
                    size_freq_calloc = self.size_freq_for_data(data, 5)
                    sizes_c, counts_c = self.sort_split_and_fill_size_freqs(size_freq_calloc)
                    sizes, counts = self.merge_size_counts(sizes, counts, sizes_c, counts_c)

                # Bin the data
                num_bins = 16
                binned_counts = [0] * 16
                curr_bin = 0
                bin_num = 0
                for i in range(max(sizes))[1:]:
                    if i % num_bins == 0 and i != 0:
                        logging.debug('Bin end: %d' % i)
                        binned_counts[bin_num] = curr_bin
                        # Extra logging. Enable this if you want to output the
                        # counts in each bin to the console.
                        #logging.debug('  Count: %d' % curr_bin)
                        bin_num = bin_num + 1
                        curr_bin = 0
                    if self.configuration.report_type == "nano_request_bins_ysize":
                        curr_bin = curr_bin + (counts[i-1] * sizes[i-1])
                    else:
                        curr_bin = curr_bin + counts[i-1]

                # Draw the plot
                ax = plt.subplot(plt_config)
                plt_config += 1
                if self.configuration.report_type == "nano_request_bins" or self.configuration.report_type == "nano_request_bins_ysize":
                    ax.bar(range(num_bins), binned_counts)
                    ax.set_xticks(range(num_bins))
                    x_labels = range(1, 256, 16)
                    x_labels.append("0")
                    ax.set_xticklabels(x_labels)
                    ax.set_xlabel("Request size (bytes)", fontsize=12)
                    if self.configuration.report_type == "nano_request_bins_ysize":
                        ax.set_ylabel("Total Requested (Bytes)")
                    else:
                        ax.set_ylabel("Frequency")
                    ax.set_title(call_name)
                else:
                    plt.bar(sizes, counts)

                plt.suptitle(self.configuration.fileV1)
                plt.subplots_adjust(hspace=0.5)


class Tool(object):

    def __init__(self, args):
        self.args = args

    def run(self):
        logging.debug('Loading JSON')
        configuration = ReportConfiguration.configuration_for_arguments(self.args)
        plotter_class = configuration.plotter_class()
        plotter = plotter_class(configuration)
        report_data = ReportData(self.args.fileV1, self.args.fileV2)
        plotter.plot(report_data)

        if self.args.show_plot:
            plotter.show()
        else:
            plotter.write_to_path(self.args.output)

    @classmethod
    def main(cls):
        parser = argparse.ArgumentParser(description='Analyze libmalloc_replay perfdata output. This takes as input a .pdj file containing request sizes and instruction counts and outputs various plots.')
        parser.add_argument('fileV1', help='Path to nano V1 data JSON file')
        parser.add_argument('fileV2', nargs='?', help='Optional path to nano V2 data JSON file')
        parser.add_argument('--report', dest='report_type', choices=['instructions', 'scatter', 'request_sizes', 'nano_request_bins', 'nano_request_bins_ysize'], default='instructions', help='The report type to produce (default: %(default)s)')
        parser.add_argument('--call', dest='call', default='malloc', choices=['malloc', 'calloc', 'realloc', 'memalign', 'valloc'], help="The call to analyze (default: %(default)s)")
        parser.add_argument('-f', '--xfilter', type=int, default=0, help="Filter the histogram to a range from 0 to <%(dest)s)>")
        parser.add_argument('-b', '--num_bins', type=int, default=10000, help="The number of bins to use for histogrammed data (default: %(default)s)")
        parser.add_argument('-n', '--nano_malloc_cutoff', type=int, default=256, help="The cutoff size to filter for (default: %(default)s bytes)")
        parser.add_argument('--merge_calloc', action='store_true', default=False, help='Merge calloc calls with malloc. For use with the nano_request_bins, nano_request_bins_ysize and request_sizes reports.')
        parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose debug logging')
        output_group = parser.add_mutually_exclusive_group(required=True)
        output_group.add_argument('-s', '--show_plot', action='store_true')
        output_group.add_argument('-o', '--output', default='fig.pdf', help="The output file path, including type extension (default: %(default)s)")

        args = parser.parse_args()
        if args.verbose:
            logging.basicConfig(level=logging.DEBUG)

        cls(args).run()


if __name__ == "__main__":
    Tool.main()

